// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContainerListener.h"
#include "SLEntitiesManager.h"
#include "SLManipulatorListener.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Tags.h"

// Sets default values for this component's properties
USLContainerListener::USLContainerListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	CurrGraspedObj = nullptr;
	GraspTime = -1.f;
}

// Dtor
USLContainerListener::~USLContainerListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLContainerListener::Init()
{
	if (!bIsInit)
	{
		// Init the semantic entities manager
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(GetWorld());
		}

		// Check that the owner is part of the semantic entities
		SemanticOwner = FSLEntitiesManager::GetInstance()->GetEntity(GetOwner());
		if (!SemanticOwner.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not semantically annotated.."), *FString(__func__), __LINE__);
			return false;
		}

		bIsInit = true;
		return true;
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLContainerListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if(USLManipulatorListener* Sibling = CastChecked<USLManipulatorListener>(
			GetOwner()->GetComponentByClass(USLManipulatorListener::StaticClass())))
		{
			Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLContainerListener::OnSLGraspBegin);
			Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLContainerListener::OnSLGraspEnd);

			bIsStarted = true;
		}
		else
		{	
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find sibling USLManipulatorListener .."), *FString(__func__), __LINE__);
		}
	}
}

// Finish active events
void USLContainerListener::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Finish any active event
		FinishActiveEvents();

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}


// Called when grasp starts
void USLContainerListener::OnSLGraspBegin(const FSLEntity& Self, AActor* Other, float Time, const FString& GraspType)
{
	if(CurrGraspedObj)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Cannot set %s as grasped object.. manipulator is already grasping %s;"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
		return;
	}
	
	// Mark as grasped
	CurrGraspedObj = Other;
	GraspTime = Time;

	SetContainersAndDistances();
}

// Called when grasp ends
void USLContainerListener::OnSLGraspEnd(const FSLEntity& Self, AActor* Other, float Time)
{
	if(CurrGraspedObj == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.. currently grasped object is nullptr while ending grasp with %s"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
		return;
	}

	if(CurrGraspedObj == Other)
	{

		// Publish close/open events
		for(const auto Pair : ContainerToDistance)
		{
			const float CurrDistance = FVector::Distance(Pair.Key->GetActorLocation(), CurrGraspedObj->GetActorLocation());

			if(CurrDistance - Pair.Value > MinDistance)
			{
				OnContainerManipulation.Broadcast(SemanticOwner, Pair.Key, GraspTime, Time, "open");
			}
			else if(CurrDistance - Pair.Value < - MinDistance)
			{
				OnContainerManipulation.Broadcast(SemanticOwner, Pair.Key, GraspTime, Time, "close");
			}
		}
		
		// Mark as released, empty previous container
		CurrGraspedObj = nullptr;
		ContainerToDistance.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] End grasp with %s while %s is still grasped.. ignoring event.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
	}
}

// Search which container will be manipulated and save their current distance to the grasped item
bool USLContainerListener::SetContainersAndDistances()
{
	// Get the outermost attachment parent of the actor
	const auto GetOutermostAttachmentLambda = [](AActor* Actor)
	{
		if(AActor* OutermostAttachParent = Actor->GetAttachParentActor())
		{
			while(AActor* CurrAttParent = OutermostAttachParent->GetAttachParentActor())
			{
				OutermostAttachParent = CurrAttParent;
			}
			return OutermostAttachParent;
		}
		return (AActor*)nullptr;
	};

	// Set of all the other actors connected with constraints
	// (containers can only be linked through constrained actors, since otherwise they would be moving together)
	TSet<AActor*> OtherConstraintActors;
	// Get outermost attachment
	if(AActor* OutermostAttachParent = GetOutermostAttachmentLambda(CurrGraspedObj))
	{
		GetAllConstraintsOtherActors(OutermostAttachParent, OtherConstraintActors);
	}
	else
	{
		GetAllConstraintsOtherActors(CurrGraspedObj, OtherConstraintActors);
	}

	// Set of the found containers
	TSet<AActor*> Containers;
	// TODO recurse over all constraint chain links, we now stop at the second link
	// Iterate the other constraint and search for containers
	for(const auto& OtherConstrAct : OtherConstraintActors)
	{
		// Search for containers starting with the outermost
		if(AActor* OutermostAttachParent = GetOutermostAttachmentLambda(OtherConstrAct))
		{
			GetAllAttachedContainers(OutermostAttachParent, Containers);
		}
		else
		{
			GetAllAttachedContainers(OtherConstrAct, Containers);
		}
	}

	// Store the containers and their distances to the manipulator
	for(const auto& C : Containers)
	{
		ContainerToDistance.Emplace(C, FVector::Distance(C->GetActorLocation(), CurrGraspedObj->GetActorLocation()));
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Container=%s; Dist=%f"),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *C->GetName(), FVector::Distance(C->GetActorLocation(), CurrGraspedObj->GetActorLocation()));
	}


	return Containers.Num() > 0;
}

// Finish any active events
void USLContainerListener::FinishActiveEvents()
{
	if(CurrGraspedObj)
	{
		// Fake a grasp end call
		OnSLGraspEnd(SemanticOwner, CurrGraspedObj, GetWorld()->GetTimeSeconds());
	}	
}

// Iterate recursively attached constraints actors of parent, append other constrained actors to set
void USLContainerListener::GetAllConstraintsOtherActors(AActor* Actor, TSet<AActor*>& OutOtherConstraintActors)
{
	if(APhysicsConstraintActor* AttAsPCA = Cast<APhysicsConstraintActor>(Actor))
	{
		if(AttAsPCA->GetConstraintComp()->ConstraintActor1 != Actor)
		{
			OutOtherConstraintActors.Emplace(AttAsPCA->GetConstraintComp()->ConstraintActor1);
		}
		else if(AttAsPCA->GetConstraintComp()->ConstraintActor2 != Actor)
		{
			OutOtherConstraintActors.Emplace(AttAsPCA->GetConstraintComp()->ConstraintActor2);
		}
	}

	TArray<AActor*> AttActors;
	Actor->GetAttachedActors(AttActors);
	for(const auto& AttAct : AttActors)
	{
		if(APhysicsConstraintActor* AttAsPCA = Cast<APhysicsConstraintActor>(AttAct))
		{
			if(AttAsPCA->GetConstraintComp()->ConstraintActor1 != Actor)
			{
				OutOtherConstraintActors.Emplace(AttAsPCA->GetConstraintComp()->ConstraintActor1);
			}
			else if(AttAsPCA->GetConstraintComp()->ConstraintActor2 != Actor)
			{
				OutOtherConstraintActors.Emplace(AttAsPCA->GetConstraintComp()->ConstraintActor2);
			}
		}

		GetAllConstraintsOtherActors(AttAct, OutOtherConstraintActors);
	}
}

// Iterate recursively on the attached actors, and search for container type
void USLContainerListener::GetAllAttachedContainers(AActor* Actor, TSet<AActor*>& OutContainers)
{
	if(FTags::HasKey(Actor, "SemLog", "Container"))
	{
		OutContainers.Emplace(Actor);
	}

	TArray<AActor*> AttActors;
	Actor->GetAttachedActors(AttActors);
	for(const auto& AttAct : AttActors)
	{
		if(FTags::HasKey(AttAct, "SemLog", "Container"))
		{
			OutContainers.Emplace(AttAct);
		}

		GetAllConstraintsOtherActors(AttAct, OutContainers);
	}
}
