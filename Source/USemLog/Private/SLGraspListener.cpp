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
	InputAxisName = "LeftGrasp";
	GraspCheckUpdateRate = 0.2f;

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
				// Check if the grasp check should be synced with the input callback
				if (GraspCheckUpdateRate > 0.f)
				{
					// Un-synced
					IC->BindAxis(InputAxisName, this, &USLGraspListener::InputAxisCallback);
					GetWorld()->GetTimerManager().SetTimer(GraspTimerHandle, this,
						&USLGraspListener::GraspCheckUpdate, GraspCheckUpdateRate, true);
				}
				else
				{
					// Synced
					IC->BindAxis(InputAxisName, this, &USLGraspListener::InputAxisCallbackWithGraspCheck);
				}
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

// Start listening to grasp events, update currently overlapping objects
void USLGraspListener::Idle(bool bInIdle)
{
	if (bIsStarted && bInIdle != bIsIdle)
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
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLGraspListener::LoadOverlapGroups()
{
	// Check grasp overlap components of owner and add them to their groups
	TArray<UActorComponent*> GraspOverlaps = GetOwner()->GetComponentsByClass(USLGraspOverlapShape::StaticClass());
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
	if (Value > 0.3)
	{
		Idle(false);
	}
	else
	{	
		Idle(true);
	}
}

// Check if the grasp trigger is active with grasp check synced
void USLGraspListener::InputAxisCallbackWithGraspCheck(float Value)
{
	if (Value > 0.3)
	{
		Idle(false);
	}
	else
	{
		Idle(true);
	}

	GraspCheckUpdate();
}

// Check if the grasp trigger is active
void USLGraspListener::GraspCheckUpdate()
{
	// TODO add a bChanged flag
	if (!bIsIdle)
	{
		TSet<AActor*> UnionAB = SetA.Union(SetB);
		for (const auto& Obj : UnionAB)
		{
			if (!GraspedObjects.Contains(Obj))
			{
				UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Grasped Obj %s at %f"),
					TEXT(__func__), __LINE__, *Obj->GetName(),GetWorld()->GetTimeSeconds());
				GraspedObjects.Emplace(Obj);
			}

		}
	}
	else
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

// Process beginning of contact in group A
void USLGraspListener::OnBeginGroupAContact(AActor* OtherActor)
{
	SetA.Emplace(OtherActor);
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Other=%s"), TEXT(__func__), __LINE__, *OtherActor->GetName());
}

// Process ending of contact in group A
void USLGraspListener::OnEndGroupAContact(AActor* OtherActor)
{
	SetA.Remove(OtherActor);
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Other=%s"), TEXT(__func__), __LINE__, *OtherActor->GetName());
}

// Process beginning of contact in group B
void USLGraspListener::OnBeginGroupBContact(AActor* OtherActor)
{
	SetB.Emplace(OtherActor);
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Other=%s"), TEXT(__func__), __LINE__, *OtherActor->GetName());
}

// Process ending of contact in group B
void USLGraspListener::OnEndGroupBContact(AActor* OtherActor)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Other=%s"), TEXT(__func__), __LINE__, *OtherActor->GetName());
	SetB.Remove(OtherActor);
}