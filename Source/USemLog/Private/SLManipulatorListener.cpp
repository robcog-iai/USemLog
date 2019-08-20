// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManipulatorListener.h"
#include "SLManipulatorOverlapSphere.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLEntitiesManager.h"
#include "GameFramework/PlayerController.h"
#if SL_WITH_MC_GRASP
#include "MCGraspAnimController.h"
#endif // SL_WITH_MC_GRASP


// Sets default values for this component's properties
USLManipulatorListener::USLManipulatorListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bGraspIsDirty = true;
	bIsPaused = false;
	InputAxisName = "LeftGrasp";
	bIsNotSkeletal = false;
	UnPauseTriggerVal = 0.5;
	
#if WITH_EDITOR
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITOR

	ActiveGraspType = "Default";
}

// Dtor
USLManipulatorListener::~USLManipulatorListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLManipulatorListener::Init(bool bInDetectGrasps, bool bInDetectContacts)
{
	if (!bIsInit)
	{
		bDetectGrasps = bInDetectGrasps;
		bDetectContacts = bInDetectContacts;
		
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

		// Remove any unset references in the array
		Fingers.Remove(nullptr);

#if SL_WITH_MC_GRASP
		// Subscribe to grasp type changes
		 SubscribeToGraspTypeChanges();
#endif // SL_WITH_MC_GRASP
		

		// True if each group has at least one bone overlap
		if (LoadOverlapGroups())
		{
			for (auto BoneOverlap : GroupA)
			{
				BoneOverlap->Init(bDetectGrasps, bDetectContacts);
			}
			for (auto BoneOverlap : GroupB)
			{
				BoneOverlap->Init(bDetectGrasps, bDetectContacts);
			}

			bIsInit = true;
			return true;
		}
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLManipulatorListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Bind grasp trigger input and update check functions
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (UInputComponent* IC = PC->InputComponent)
			{
				IC->BindAxis(InputAxisName, this, &USLManipulatorListener::InputAxisCallback);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d No Input Component found.."), *FString(__func__), __LINE__);
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No Player controller found.."), *FString(__func__), __LINE__);
			return;
		}

		// Start listening on the bone overlaps
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->Start();
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnBeginContact);
				BoneOverlap->OnEndSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnEndContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnBeginGroupAGraspContact);
				BoneOverlap->OnEndSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnEndGroupAGraspContact);
			}
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Start();
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnBeginContact);
				BoneOverlap->OnEndSLContactOverlap.AddUObject(this, &USLManipulatorListener::OnEndContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnBeginGroupBGraspContact);
				BoneOverlap->OnEndSLGraspOverlap.AddUObject(this, &USLManipulatorListener::OnEndGroupBGraspContact);
			}
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue grasp detection
void USLManipulatorListener::Pause(bool bInPause)
{
	if (bInPause != bIsPaused)
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->PauseGrasp(bInPause);
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->PauseGrasp(bInPause);
		}
		bIsPaused = bInPause;


		// Clear sets
		if (bInPause)
		{
			for (const auto& Obj : GraspedObjects)
			{
				OnEndManipulatorGrasp.Broadcast(SemanticOwner, Obj, GetWorld()->GetTimeSeconds());
			}
			GraspedObjects.Empty();
			SetA.Empty();
			SetB.Empty();
		}
	}
}

// Stop publishing grasp events
void USLManipulatorListener::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLManipulatorListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorListener, HandType))
	{
		if (HandType == ESLGraspHandType::Left)
		{
			InputAxisName = "LeftGrasp";
		}
		else if (HandType == ESLGraspHandType::Right)
		{
			InputAxisName = "RightGrasp";
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorListener, bIsNotSkeletal))
	{
		Fingers.Empty();
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLManipulatorListener::LoadOverlapGroups()
{
	// Lambda to check grasp overlap components of owner and add them to their groups
	auto GetOverlapComponentsLambda = [this](AActor* Owner)
	{
		TArray<UActorComponent*> GraspOverlaps = Owner->GetComponentsByClass(USLManipulatorOverlapSphere::StaticClass());
		for (UActorComponent* GraspOverlapComp : GraspOverlaps)
		{
			USLManipulatorOverlapSphere* GraspOverlap = CastChecked<USLManipulatorOverlapSphere>(GraspOverlapComp);
			if (GraspOverlap->Group == ESLManipulatorOverlapGroup::A)
			{
				GroupA.Add(GraspOverlap);
			}
			else if (GraspOverlap->Group == ESLManipulatorOverlapGroup::B)
			{
				GroupB.Add(GraspOverlap);
			}
		}
	};

	if (bIsNotSkeletal)
	{
		for (const auto& F : Fingers)
		{
			GetOverlapComponentsLambda(F);
		}
	}
	else 
	{
		GetOverlapComponentsLambda(GetOwner());
	}

	// Check if at least one valid overlap shape is in each group
	if (GroupA.Num() == 0 || GroupB.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d One of the grasp groups is empty grasp detection disabled."), *FString(__func__), __LINE__);
		bDetectGrasps = false;
	}
	return true;
}

#if SL_WITH_MC_GRASP
// Subscribe to grasp type changes
bool USLManipulatorListener::SubscribeToGraspTypeChanges()
{
	if (UMCGraspAnimController* Sibling = CastChecked<UMCGraspAnimController>(
		GetOwner()->GetComponentByClass(UMCGraspAnimController::StaticClass())))
	{
		Sibling->OnGraspType.AddUObject(this, &USLManipulatorListener::OnGraspType);
		return true;
	}
	return false;
}

// Callback on grasp type change
void USLManipulatorListener::OnGraspType(const FString& Type)
{
	ActiveGraspType = Type;
	ActiveGraspType.RemoveFromStart("GA_");
	ActiveGraspType.RemoveFromEnd("_Left");
	ActiveGraspType.RemoveFromEnd("_Right");
	ActiveGraspType.Append("Grasp");
	UE_LOG(LogTemp, Warning, TEXT("%s::%d ActiveGraspType=%s"), *FString(__func__), __LINE__, *ActiveGraspType);
}
#endif // SL_WITH_MC_GRASP

// Check if the grasp trigger is active
void USLManipulatorListener::InputAxisCallback(float Value)
{
	if (Value >= UnPauseTriggerVal)
	{
		Pause(false);
	}
	else
	{	
		Pause(true);
	}
}

// Check for grasping state
void USLManipulatorListener::CheckGraspState()
{
	for (const auto& Obj : SetA.Intersect(SetB))
	{
		if (!GraspedObjects.Contains(Obj))
		{
			BeginGrasp(Obj);
		}
	}
}

// A grasp has started
void USLManipulatorListener::BeginGrasp(AActor* OtherActor)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue,
	//	FString::Printf(TEXT(" * * * * *BEGIN* *BCAST* *Begin Grasp* %s"),
	//		*OtherActor->GetName()), false, FVector2D(1.5f, 1.5f));
	GraspedObjects.Emplace(OtherActor);
	OnBeginManipulatorGrasp.Broadcast(SemanticOwner, OtherActor, GetWorld()->GetTimeSeconds(), ActiveGraspType);
}

// A grasp has ended
void USLManipulatorListener::EndGrasp(AActor* OtherActor)
{
	if (GraspedObjects.Remove(OtherActor) > 0)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue,
		//	FString::Printf(TEXT(" * * * * *BEGIN* *BCAST* *End Grasp* %s"),
		//		*OtherActor->GetName()), false, FVector2D(1.5f, 1.5f));
		OnEndManipulatorGrasp.Broadcast(SemanticOwner, OtherActor, GetWorld()->GetTimeSeconds());
	}
}

// All grasps have ended
void USLManipulatorListener::EndAllGrasps()
{
	for (const auto& Obj : GraspedObjects)
	{
		OnEndManipulatorGrasp.Broadcast(SemanticOwner, Obj, GetWorld()->GetTimeSeconds());
	}
	GraspedObjects.Empty();
}

// Process beginning of contact in group A
void USLManipulatorListener::OnBeginGroupAGraspContact(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetA.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupB.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}
}

// Process beginning of contact in group B
void USLManipulatorListener::OnBeginGroupBGraspContact(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetB.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupA.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}
}

// Process beginning of contact
void USLManipulatorListener::OnBeginContact(AActor* OtherActor)
{
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherActor);
	if (OtherItem.IsSet())
	{
		if (int32* NumContacts = ContactObjects.Find(OtherActor))
		{
			(*NumContacts)++;
		}
		else
		{
			ContactObjects.Add(OtherActor, 1);
			// Broadcast begin of semantic overlap event
			FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
				GetWorld()->GetTimeSeconds(), false);
			OnBeginManipulatorContact.Broadcast(SemanticOverlapResult);
		}
	}
}

// Process ending of contact in group A
void USLManipulatorListener::OnEndGroupAGraspContact(AActor* OtherActor)
{
	if (SetA.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			EndGrasp(OtherActor);
		}
	}
}

// Process ending of contact in group B
void USLManipulatorListener::OnEndGroupBGraspContact(AActor* OtherActor)
{
	if (SetB.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			EndGrasp(OtherActor);
		}
	}
}

// Process ending of contact
void USLManipulatorListener::OnEndContact(AActor* OtherActor)
{
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherActor);
	if (OtherItem.IsSet())
	{
		if (int32* NumContacts = ContactObjects.Find(OtherActor))
		{
			(*NumContacts)--;
			if ((*NumContacts) < 1)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue,
				//	FString::Printf(TEXT(" * * * * *END* *BCAST* *HAND CONTACT* %s"),
				//		*OtherActor->GetName()), false, FVector2D(1.5f, 1.5f));
				OnEndManipulatorContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, GetWorld()->GetTimeSeconds());
				ContactObjects.Remove(OtherActor);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}