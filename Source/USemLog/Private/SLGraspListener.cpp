// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLGraspListener.h"
#include "Animation/SkeletalMeshActor.h"

// Sets default values for this component's properties
USLGraspListener::USLGraspListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bGraspIsDirty = true;
	bCheckForContacts = true;
	InputAxisName = "LeftGrasp";
	bIsNotSkeletal = false;
	IdleWakeupValue = 0.5;

#if WITH_EDITOR
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITOR
}

// Dtor
USLGraspListener::~USLGraspListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLGraspListener::Init()
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

		// Remove any unset references in the array
		Fingers.Remove(nullptr);

		// True if each group has at least one bone overlap
		if (LoadOverlapGroups())
		{
			for (auto BoneOverlap : GroupA)
			{
				BoneOverlap->Init();
			}
			for (auto BoneOverlap : GroupB)
			{
				BoneOverlap->Init();
			}

			bIsInit = true;
			return true;
		}
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLGraspListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Bind grasp trigger input and update check functions
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (UInputComponent* IC = PC->InputComponent)
			{
				IC->BindAxis(InputAxisName, this, &USLGraspListener::InputAxisCallback);
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
			BoneOverlap->OnBeginSLGraspOverlap.AddUObject(this, &USLGraspListener::OnBeginGroupAContact);
			BoneOverlap->OnEndSLGraspOverlap.AddUObject(this, &USLGraspListener::OnEndGroupAContact);
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Start();
			BoneOverlap->OnBeginSLGraspOverlap.AddUObject(this, &USLGraspListener::OnBeginGroupBContact);
			BoneOverlap->OnEndSLGraspOverlap.AddUObject(this, &USLGraspListener::OnEndGroupBContact);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue grasp detection
void USLGraspListener::Idle(bool bInIdle)
{
	if (bInIdle != bIsIdle)
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->Idle(bInIdle);
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Idle(bInIdle);
		}
		bIsIdle = bInIdle;


		// Clear sets
		if (bInIdle)
		{
			for (const auto& Obj : GraspedObjects)
			{
				UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Released Obj %s at %f"),
					TEXT(__func__), __LINE__, *Obj->GetName(), GetWorld()->GetTimeSeconds());
			}
			GraspedObjects.Empty();
			SetA.Empty();
			SetB.Empty();
		}
	}
}

// Stop publishing grasp events
void USLGraspListener::Finish(bool bForced)
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
void USLGraspListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspListener, HandType))
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspListener, bIsNotSkeletal))
	{
		Fingers.Empty();
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLGraspListener::LoadOverlapGroups()
{
	// Lambda to check grasp overlap components of owner and add them to their groups
	auto GetOverlapComponentsLambda = [this](AActor* Owner)
	{
		TArray<UActorComponent*> GraspOverlaps = Owner->GetComponentsByClass(USLGraspOverlapShape::StaticClass());
		for (UActorComponent* GraspOverlapComp : GraspOverlaps)
		{
			USLGraspOverlapShape* GraspOverlap = CastChecked<USLGraspOverlapShape>(GraspOverlapComp);
			if (GraspOverlap->Group == ESLGraspOverlapGroup::A)
			{
				GroupA.Add(GraspOverlap);
			}
			else if (GraspOverlap->Group == ESLGraspOverlapGroup::B)
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d One of the grasp groups is empty."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Check if the grasp trigger is active
void USLGraspListener::InputAxisCallback(float Value)
{
	if (Value > IdleWakeupValue)
	{
		Idle(false);
	}
	else
	{	
		Idle(true);
	}
}

// Check for grasping state
void USLGraspListener::CheckGraspState()
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
void USLGraspListener::BeginGrasp(AActor* Other)
{
	GraspedObjects.Emplace(Other);
	OnBeginSLGrasp.Broadcast(SemanticOwner, Other, GetWorld()->GetTimeSeconds());
}

// A grasp has ended
void USLGraspListener::EndGrasp(AActor* Other)
{
	if (GraspedObjects.Remove(Other) > 0)
	{
		OnEndSLGrasp.Broadcast(SemanticOwner, Other, GetWorld()->GetTimeSeconds());
	}
}

// All grasps have ended
void USLGraspListener::EndAllGrasps()
{
	for (const auto& Obj : GraspedObjects)
	{
		OnEndSLGrasp.Broadcast(SemanticOwner, Obj, GetWorld()->GetTimeSeconds());
	}
	GraspedObjects.Empty();
}

// Check for begin contact
void USLGraspListener::CheckBeginContact(AActor* Other)
{
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(Other);
	if (!OtherItem.IsSet())
	{
		if (int32* NumContacts = ContactObjects.Find(Other))
		{
			*NumContacts++;
		}
		else
		{
			ContactObjects.Add(Other, 1);
			// Broadcast begin of semantic overlap event
			FSLContactOverlapResult SemanticOverlapResult(SemanticOwner, OtherItem,
				GetWorld()->GetTimeSeconds(), false);
			OnBeginSLOverlap.Broadcast(SemanticOverlapResult);
		}
	}
}

// Check for begin contact
void USLGraspListener::CheckEndContact(AActor* Other)
{
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(Other);
	if (!OtherItem.IsSet())
	{
		if (int32* NumContacts = ContactObjects.Find(Other))
		{
			*NumContacts--;
			if (NumContacts == 0)
			{
				OnEndSLOverlap.Broadcast(SemanticOwner.Obj, OtherItem.Obj, GetWorld()->GetTimeSeconds());
			}
			ContactObjects.Remove(Other);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}

// Process beginning of contact in group A
void USLGraspListener::OnBeginGroupAContact(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetA.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupB.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}

	if (bCheckForContacts)
	{
		CheckBeginContact(OtherActor);
	}
}

// Process beginning of contact in group B
void USLGraspListener::OnBeginGroupBContact(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetB.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupA.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}

	if (bCheckForContacts)
	{
		CheckBeginContact(OtherActor);
	}
}

// Process ending of contact in group A
void USLGraspListener::OnEndGroupAContact(AActor* OtherActor)
{
	if (SetA.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			EndGrasp(OtherActor);
		}
	}

	if (bCheckForContacts)
	{
		CheckEndContact(OtherActor);
	}
}

// Process ending of contact in group B
void USLGraspListener::OnEndGroupBContact(AActor* OtherActor)
{
	if (SetB.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			EndGrasp(OtherActor);
		}
	}

	if (bCheckForContacts)
	{
		CheckEndContact(OtherActor);
	}
}