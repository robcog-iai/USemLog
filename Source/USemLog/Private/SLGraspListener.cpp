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

	UE_LOG(LogTemp, Error, TEXT("%s::%d"), *FString(__func__), __LINE__);

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Default values
#if WITH_EDITOR
	HandType = ESLGraspHandType::Left;
	bAddDefaultBoneTypes = false;
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
		// Make sure the semantic entities are set
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(GetWorld());
		}

		//  Check if owner is part of the semantic entities
		SemanticOwner = FSLEntitiesManager::GetInstance()->GetEntity(GetOwner());
		if (!SemanticOwner.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not semantically annotated.."), *FString(__func__), __LINE__);
			return bIsInit;
		}

		// True if each group has at least one bone overlap
		bIsInit = SetOverlapGroups();
	}
	return bIsInit;
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

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLGraspListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set the left / right constraint actors
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLGraspListener, bAddDefaultBoneTypes))
	{
		AddDefaultBoneNames();
		bAddDefaultBoneTypes = false;
	}
}


// Add default bone names
void USLGraspListener::AddDefaultBoneNames()
{
	// Clear any previous bone names
	BoneNamesGroupA.Empty();
	BoneNamesGroupB.Empty();
	
	if (HandType == ESLGraspHandType::Left)
	{
		BoneNamesGroupA.Add("index_03_l");
		BoneNamesGroupA.Add("middle_03_l");
		BoneNamesGroupA.Add("pinky_03_l");
		BoneNamesGroupA.Add("ring_03_l");

		BoneNamesGroupB.Add("thumb_03_l");
	}
	else if (HandType == ESLGraspHandType::Right)
	{
		BoneNamesGroupA.Add("index_03_r");
		BoneNamesGroupA.Add("middle_03_r");
		BoneNamesGroupA.Add("pinky_03_r");
		BoneNamesGroupA.Add("ring_03_r");

		BoneNamesGroupB.Add("thumb_03_r");
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLGraspListener::SetOverlapGroups()
{
	// Check if owner is a skeletal actor
	if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(GetOwner()))
	{
		// Get the skeletal mesh component
		if (USkeletalMeshComponent* SMC = SkelAct->GetSkeletalMeshComponent())
		{
			// Lambda to create and attach a bone overlap shape, returns true if shape successfully attached
			auto CreateAndAttachBoneOverlapShapeLambda = [this, &SMC](const FName& BoneName, TArray<USLBoneOverlapShape*>& Group) -> bool
			{
				// Check if bone name is part of the skeleton
				if (SMC->GetBoneIndex(BoneName) == INDEX_NONE)
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not part of the skeleton"),
						*FString(__func__), __LINE__, *BoneName.ToString());
					return false;
				}

				// Create new bone overlap and attach it to the bone
				USLBoneOverlapShape* BoneOverlapShape = NewObject<USLBoneOverlapShape>(this, BoneName);
				if (BoneOverlapShape->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName))
				{
					if (BoneOverlapShape->Init(SMC, BoneName))
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Successfull init and attachment to %s"),
							*FString(__func__), __LINE__, *BoneName.ToString());
						Group.Emplace(BoneOverlapShape);
						return true;
					}
				}

				UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to attach or to init bone shape %s"),
					*FString(__func__), __LINE__, *BoneName.ToString());
				BoneOverlapShape->DestroyComponent();
				return false;
			};

			// Create and attach overlap events for Group A
			for (const auto& BoneName : BoneNamesGroupA)
			{
				CreateAndAttachBoneOverlapShapeLambda(BoneName, GroupA);

				//// Create a new overlap component
				//USLBoneOverlapShape* BoneOverlapShape = NewObject<USLBoneOverlapShape>(this, BoneName);

				//// Attach to component bone
				//if (BoneOverlapShape->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName))
				//{
				//	BoneOverlapShape->Init(SMC, BoneName);
				//	UE_LOG(LogTemp, Warning, TEXT("%s::%d GroupA: Successfull attachment to %s"),
				//		*FString(__func__), __LINE__, *BoneName.ToString());
				//	GroupA.Emplace(BoneOverlapShape);
				//}
				//else
				//{
				//	UE_LOG(LogTemp, Error, TEXT("%s::%d GroupA: Faild to attach to %s"),
				//		*FString(__func__), __LINE__, *BoneName.ToString());
				//}
			}

			// Check if at least one valid overlap shape is in the group
			if (GroupA.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d GroupA is empty."), *FString(__func__), __LINE__);
				return false;
			}

			// Create and attach overlap events for Group B
			for (const auto& BoneName : BoneNamesGroupB)
			{
				CreateAndAttachBoneOverlapShapeLambda(BoneName, GroupB);

				//// Create a new overlap component
				//USLBoneOverlapShape* BoneOverlapShape = NewObject<USLBoneOverlapShape>(this, BoneName);

				//// Attach to component bone
				//if (BoneOverlapShape->AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName))
				//{
				//	BoneOverlapShape->Init(SMC, BoneName);
				//	UE_LOG(LogTemp, Warning, TEXT("%s::%d GroupA: Successfull attachment to %s"),
				//		*FString(__func__), __LINE__, *BoneName.ToString());
				//	GroupB.Emplace(BoneOverlapShape);
				//}
				//else
				//{
				//	UE_LOG(LogTemp, Error, TEXT("%s::%d GroupA: Faild to attach to %s"),
				//		*FString(__func__), __LINE__, *BoneName.ToString());
				//}
			}

			// Check if at least one valid overlap shape is in the group
			if (GroupB.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d GroupB is empty."), *FString(__func__), __LINE__);
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not a skeletal mesh actor"), *FString(__func__), __LINE__);
		return false;
	}
}
