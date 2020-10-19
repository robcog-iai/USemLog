// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLContainerMonitor.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"

#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


// Sets default values for this component's properties
USLContainerMonitor::USLContainerMonitor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	CurrGraspedIndividual = nullptr;
	GraspTime = -1.f;
}

// Dtor
USLContainerMonitor::~USLContainerMonitor()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLContainerMonitor::Init()
{
	if (!bIsInit)
	{
		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			OwnerIndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!OwnerIndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return false;
		}

		// Set the individual object
		OwnerIndividualObject = OwnerIndividualComponent->GetIndividualObject();

		bIsInit = true;
		return true;
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLContainerMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if(USLManipulatorMonitor* Sibling = CastChecked<USLManipulatorMonitor>(
			GetOwner()->GetComponentByClass(USLManipulatorMonitor::StaticClass())))
		{
			Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLContainerMonitor::OnSLGraspBegin);
			Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLContainerMonitor::OnSLGraspEnd);

			bIsStarted = true;
		}
		else
		{	
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find sibling USLManipulatorMonitor .."), *FString(__func__), __LINE__);
		}
	}
}

// Finish active events
void USLContainerMonitor::Finish(bool bForced)
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
void USLContainerMonitor::OnSLGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& GraspType)
{
	if(CurrGraspedIndividual)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Cannot set %s as grasped object.. manipulator is already grasping %s;"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
			*Other->GetParentActor()->GetName(), *CurrGraspedIndividual->GetParentActor()->GetName());
		return;
	}
	
	// Mark as grasped
	CurrGraspedIndividual = Other;
	GraspTime = Time;

	SetContainersAndDistances();
}

// Called when grasp ends
void USLContainerMonitor::OnSLGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	if(CurrGraspedIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.. currently grasped object is nullptr while ending grasp with %s"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
		return;
	}

	if(CurrGraspedIndividual == Other)
	{
		// Publish close/open events
		for(const auto Pair : ContainerToDistance)
		{
			const float CurrDistance = FVector::Distance(Pair.Key->GetParentActor()->GetActorLocation(),
				CurrGraspedIndividual->GetParentActor()->GetActorLocation());

			if(CurrDistance - Pair.Value > MinDistance)
			{
				if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(Pair.Key))
				{
					OnContainerManipulation.Broadcast(OwnerIndividualObject, BI, GraspTime, Time, "open");
				}
			}
			else if(CurrDistance - Pair.Value < - MinDistance)
			{
				if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(Pair.Key))
				{
					OnContainerManipulation.Broadcast(OwnerIndividualObject, BI, GraspTime, Time, "close");
				}
			}
		}
		
		// Mark as released, empty previous container
		CurrGraspedIndividual = nullptr;
		ContainerToDistance.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] End grasp with %s while %s is still grasped.. ignoring event.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
			*Other->GetParentActor()->GetName(), *CurrGraspedIndividual->GetParentActor()->GetName());
	}
}

// Search which container will be manipulated and save their current distance to the grasped item
bool USLContainerMonitor::SetContainersAndDistances()
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
	if(AActor* OutermostAttachParent = GetOutermostAttachmentLambda(CurrGraspedIndividual->GetParentActor()))
	{
		GetAllConstraintsOtherActors(OutermostAttachParent, OtherConstraintActors);
	}
	else
	{
		GetAllConstraintsOtherActors(CurrGraspedIndividual->GetParentActor(), OtherConstraintActors);
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
		ContainerToDistance.Emplace(C, FVector::Distance(C->GetActorLocation(), CurrGraspedIndividual->GetParentActor()->GetActorLocation()));
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Container=%s; Dist=%f"),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *C->GetName(), FVector::Distance(C->GetActorLocation(), CurrGraspedActor->GetActorLocation()));
	}


	return Containers.Num() > 0;
}

// Finish any active events
void USLContainerMonitor::FinishActiveEvents()
{
	if(CurrGraspedIndividual)
	{
		// Fake a grasp end call
		OnSLGraspEnd(OwnerIndividualObject, CurrGraspedIndividual, GetWorld()->GetTimeSeconds());
	}	
}

// Iterate recursively attached constraints actors of parent, append other constrained actors to set
void USLContainerMonitor::GetAllConstraintsOtherActors(AActor* Actor, TSet<AActor*>& OutOtherConstraintActors)
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
void USLContainerMonitor::GetAllAttachedContainers(AActor* Actor, TSet<AActor*>& OutContainers)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO "), *FString(__FUNCTION__), __LINE__);
	//if(FTags::HasKey(Actor, "SemLog", "Container"))
	//{
	//	OutContainers.Emplace(Actor);
	//}

	//TArray<AActor*> AttActors;
	//Actor->GetAttachedActors(AttActors);
	//for(const auto& AttAct : AttActors)
	//{
	//	if(FTags::HasKey(AttAct, "SemLog", "Container"))
	//	{
	//		OutContainers.Emplace(AttAct);
	//	}

	//	GetAllConstraintsOtherActors(AttAct, OutContainers);
	//}
}
