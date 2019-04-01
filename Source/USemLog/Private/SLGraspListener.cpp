// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLGraspListener.h"
#include "Animation/SkeletalMeshActor.h"

// Sets default values for this component's properties
USLGraspListener::USLGraspListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	UE_LOG(LogTemp, Error, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
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
void USLGraspListener::Init()
{
	if (!bIsInit)
	{
		// Make sure the semantic entities are set
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(GetWorld());
		}

		//  Check if owner is part of the semantic entities
		SemanticOwner = FSLEntitiesManager::GetInstance()->GetEntity(GetOwner());
		if (!SemanticOwner.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not semantically annotated.."), TEXT(__FUNCTION__), __LINE__);
			return;
		}

		if (SetOverlapGroups())
		{
			bIsInit = true;
		}
	}
}

// Start listening to grasp events, update currently overlapping objects
void USLGraspListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Mark as started
		bIsStarted = true;
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

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLGraspListener::SetOverlapGroups()
{
	// Check if owner is a skeletal actor
	if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		// Get the skeletal mesh component
		if (USkeletalMeshComponent* SMC = SkelAct->GetSkeletalMeshComponent())
		{
			// Create and attach overlap events for Group A
			for (const auto& BoneName : BoneNamesGroupA)
			{
				// Create a new overlap component
				USLBoneOverlapShape* BoneOverlapShape = NewObject<USLBoneOverlapShape>(this, BoneName);

				// Attach to component bone
				if (BoneOverlapShape->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d GroupA: Successfull attachment to %s"),
						TEXT(__FUNCTION__), __LINE__, *BoneName.ToString());
					GroupA.Emplace(BoneOverlapShape);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d GroupA: Faild to attach to %s"),
						TEXT(__FUNCTION__), __LINE__, *BoneName.ToString());
				}
			}

			// Check if at least one valid overlap shape is in the group
			if (GroupA.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d GroupA is empty."), TEXT(__FUNCTION__), __LINE__);
				return false;
			}

			// Create and attach overlap events for Group B
			for (const auto& BoneName : BoneNamesGroupB)
			{
				// Create a new overlap component
				USLBoneOverlapShape* BoneOverlapShape = NewObject<USLBoneOverlapShape>(this, BoneName);

				// Attach to component bone
				if (BoneOverlapShape->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d GroupA: Successfull attachment to %s"),
						TEXT(__FUNCTION__), __LINE__, *BoneName.ToString());
					GroupB.Emplace(BoneOverlapShape);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d GroupA: Faild to attach to %s"),
						TEXT(__FUNCTION__), __LINE__, *BoneName.ToString());
				}
			}

			// Check if at least one valid overlap shape is in the group
			if (GroupB.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d GroupB is empty."), TEXT(__FUNCTION__), __LINE__);
				return false;
			}

			// All well
			return true;
		}
		// All well
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not a skeletal mesh actor"), TEXT(__FUNCTION__), __LINE__);
		return false;
	}
}
