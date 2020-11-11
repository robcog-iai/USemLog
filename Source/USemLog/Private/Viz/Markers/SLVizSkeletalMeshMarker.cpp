// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Markers/SLVizSkeletalMeshMarker.h"
#include "Viz/SLVizAssets.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizSkeletalMeshMarker::USLVizSkeletalMeshMarker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PMCRef = nullptr;
}

// Called every frame, used for timeline visualizations, activated and deactivated on request
void USLVizSkeletalMeshMarker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Increase the passed time
	TimelineDeltaTime += DeltaTime;

	// Calculate the number of instances to draw depending on the passed time
	int32 NumInstancesToDraw = (TimelineDeltaTime * TimelinePoses.Num()) / TimelineDuration;

	// Wait if not enough time has passed
	if (NumInstancesToDraw == 0)
	{
		return;
	}

	// Draw all instances, or use a limit
	if (TimelineMaxNumInstances <= 0)
	{
		UpdateTimeline(NumInstancesToDraw);
	}
	else
	{
		UpdateTimelineWithMaxNumInstances(NumInstancesToDraw);
	}

	// Reset the elapsed time
	TimelineDeltaTime = 0;
}

// Set the visual properties of the skeletal mesh (use original materials)
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMesh* SkelMesh)
{
	// Create the poseable mesh reference and the dynamic material
	SetPoseableMeshComponentVisual(SkelMesh);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < PMCRef->GetNumMaterials(); ++MatIdx)
	{
		PMCRef->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Set the visual properties of the skeletal mesh
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMesh* SkelMesh, const FLinearColor& InColor, ESLVizMaterialType InMaterialType)
{
	// Create the poseable mesh reference and the dynamic material
	SetPoseableMeshComponentVisual(SkelMesh);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < PMCRef->GetNumMaterials(); ++MatIdx)
	{
		PMCRef->SetMaterial(MatIdx, DynamicMaterial);
	}
}

//// Add instances at pose
//void USLVizSkeletalMeshMarker::AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses)
//{
//	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
//		return;
//	}
//
//	UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
//	PMC->SetWorldTransform(Pose);
//
//	for (const auto& BonePosePair : BonePoses)
//	{	
//		const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
//		PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
//	}
//
//	PMCInstances.Add(PMC);
//}

// Add instance with bones
void USLVizSkeletalMeshMarker::AddInstance(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose)
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
	PMC->SetWorldTransform(SkeletalPose.Key);

	for (int32 Idx = 0; Idx < 5; ++Idx)
	{
		for (const auto& BonePosePair : SkeletalPose.Value)
		{
			const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
			PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
		}
	}

	PMCInstances.Add(PMC);
}

//// Add instances with the poses
//void USLVizSkeletalMeshMarker::AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray)
//{
//	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
//		return;
//	}
//
//	// Make sure the size of the two arrays are the same
//	const bool bIncludeBones = Poses.Num() == BonePosesArray.Num();
//	
//	int32 PoseIdx = 0;
//	for (const auto& P : Poses)
//	{
//		UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
//		PMC->SetWorldTransform(P);
//
//		if (bIncludeBones)
//		{
//			for (const auto& BonePosePair : BonePosesArray[PoseIdx])
//			{
//				const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
//				PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
//			}
//			PoseIdx++;
//		}
//
//		PMCInstances.Add(PMC);
//	}
//}

// Add instances with bone poses
void USLVizSkeletalMeshMarker::AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses)
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	for (const auto& SkelPosePair : SkeletalPoses)
	{
		UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
		PMC->SetWorldTransform(SkelPosePair.Key);

		for (int32 Idx = 0; Idx < 5; ++Idx)
		{
			for (const auto& BonePosePair : SkelPosePair.Value)
			{
				const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
				PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
			}
		}

		PMCInstances.Add(PMC);
	}
}

// Add instances with timeline update
void USLVizSkeletalMeshMarker::AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	const FSLVizTimelineParams& TimelineParams)
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (TimelineParams.Duration <= 0.008f)
	{
		AddInstances(SkeletalPoses);
		return;
	}

	if (IsComponentTickEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d A timeline is already active, reset first.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Set the timeline data
	TimelinePoses = SkeletalPoses;
	TimelineDuration = TimelineParams.Duration;
	TimelineMaxNumInstances = TimelineParams.MaxNumInstances;
	bLoopTimeline = TimelineParams.bLoop;
	TimelineIndex = 0;

	// Start timeline
	if (TimelineParams.UpdateRate > 0.f)
	{
		SetComponentTickInterval(TimelineParams.UpdateRate);
	}
	SetComponentTickEnabled(true);
}

// Unregister the component, remove it from its outer Actor's Components array and mark for pending kill
void USLVizSkeletalMeshMarker::DestroyComponent(bool bPromoteChildren)
{
	for (const auto& PMCInst : PMCInstances)
	{
		if (PMCInst && PMCInst->IsValidLowLevel() && !PMCInst->IsPendingKillOrUnreachable())
		{
			PMCInst->DestroyComponent();
		}
	}

	if (PMCRef && PMCRef->IsValidLowLevel() && !PMCRef->IsPendingKillOrUnreachable())
	{
		PMCRef->DestroyComponent();
	}

	Super::DestroyComponent(bPromoteChildren);
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizSkeletalMeshMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
}

// Reset visual related data
void USLVizSkeletalMeshMarker::ResetVisuals()
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	PMCRef->EmptyOverrideMaterials();
}

// Reset instances (poses of the visuals)
void USLVizSkeletalMeshMarker::ResetPoses()
{
	for (const auto& PMC : PMCInstances)
	{
		if (!PMC->IsPendingKillOrUnreachable())
		{
			PMC->DestroyComponent();
		}
	}
	PMCInstances.Empty();
}

//// Update intial timeline iteration (create the instances)
//void USLVizSkeletalMeshMarker::UpdateInitialTimeline(float DeltaTime)
//{
//	TimelineDeltaTime += DeltaTime;
//	const int32 NumTotalIstances = TimelinePoses.Num();
//	int32 NumInstancesToDraw = (TimelineDeltaTime * NumTotalIstances) / TimelineDuration;
//	if (NumInstancesToDraw == 0)
//	{
//		return;
//	}
//
//	if (TimelineIndex + NumInstancesToDraw < NumTotalIstances)
//	{
//		// Safe to draw all instances
//		while (NumInstancesToDraw > 0)
//		{
//			AddInstance(TimelinePoses[TimelineIndex]);
//			TimelineIndex++;
//			NumInstancesToDraw--;
//		}
//	}
//	else
//	{
//		// Decrease the num of instances to draw to avoid overflow
//		NumInstancesToDraw = NumTotalIstances - TimelineIndex;
//		while (NumInstancesToDraw > 0)
//		{
//			AddInstance(TimelinePoses[TimelineIndex]);
//			TimelineIndex++;
//			NumInstancesToDraw--;
//		}
//
//		// Check if the timeline should be repeated or stopped
//		if (bLoopTimeline)
//		{
//			HideInstances();
//			TimelinePoses.Empty();
//			TimelineIndex = 0;
//		}
//		else
//		{
//			ClearAndStopTimeline();
//		}
//	}
//}
//
//// Update loop timeline (set instaces visibility)
//void USLVizSkeletalMeshMarker::UpdateLoopTimeline(float DeltaTime)
//{
//	TimelineDeltaTime += DeltaTime;
//	const int32 NumTotalIstances = PMCInstances.Num();
//	int32 NumInstancesToDraw = (DeltaTime * NumTotalIstances) / TimelineDuration;
//	if (NumInstancesToDraw == 0)
//	{
//		return;
//	}
//
//	if (TimelineIndex + NumInstancesToDraw < NumTotalIstances)
//	{
//		while (NumInstancesToDraw > 0)
//		{
//			PMCInstances[TimelineIndex]->SetVisibility(true); // Instance is already created, set it to visible
//			TimelineIndex++;
//			NumInstancesToDraw--;
//		}
//	}
//	else
//	{
//		// Decrease the num of instances to draw to avoid overflow
//		NumInstancesToDraw = NumTotalIstances - TimelineIndex;
//		while (NumInstancesToDraw > 0)
//		{
//			PMCInstances[TimelineIndex]->SetVisibility(true); // Instance is already created, set it to visible
//			TimelineIndex++;
//			NumInstancesToDraw--;
//		}
//
//		// Check if the timeline should be repeated or stopped
//		if (bLoopTimeline)
//		{
//			HideInstances();
//			TimelineIndex = 0;
//		}
//		else
//		{
//			ClearAndStopTimeline();
//		}
//	}
//}

// Set instances visibility to false
void USLVizSkeletalMeshMarker::HideInstances()
{
	for (const auto& PMC : PMCInstances)
	{
		PMC->SetVisibility(false);
	}
}

// Clear the timeline and the related members
void USLVizSkeletalMeshMarker::ClearAndStopTimeline()
{
	if (IsComponentTickEnabled())
	{
		SetComponentTickEnabled(false);
		SetComponentTickInterval(-1.f); // Tick every frame by default (If less than or equal to 0 then it will tick every frame)
	}
	TimelineMaxNumInstances = INDEX_NONE;
	TimelineIndex = INDEX_NONE;
	TimelinePoses.Empty();
}

// Update timeline with the given number of new instances
void USLVizSkeletalMeshMarker::UpdateTimeline(int32 NumNewInstances)
{
	// Check if the instances have already been created (number of timeline poses equals the number of intances)
	bool bInstancesAlreadyCreated = TimelinePoses.Num() == PMCInstances.Num();

	// Check if it is safe to draw all new instances
	if (TimelineIndex + NumNewInstances < TimelinePoses.Num())
	{
		// Safe to draw all instances
		while (NumNewInstances > 0)
		{
			bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);
			TimelineIndex++;
			NumNewInstances--;
		}
	}
	else
	{
		// Reached end of the poses array, add only remaining values
		while (TimelinePoses.IsValidIndex(TimelineIndex))
		{
			bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);
			TimelineIndex++;
		}

		// Hide existing instances if timeline should be looped
		if (bLoopTimeline)
		{
			// Avoid destroying the instances
			HideInstances();
			// TimelinePoses.Empty(); commented out since it is used to detect if the instances were created, or create explicit flag
			TimelineIndex = 0;
		}
		else
		{
			ClearAndStopTimeline();
		}
	}
}

// Update timeline with max number of instances
void USLVizSkeletalMeshMarker::UpdateTimelineWithMaxNumInstances(int32 NumNewInstances)
{
	// Check if the instances have already been created (number of timeline poses equals the number of intances)
	bool bInstancesAlreadyCreated = TimelinePoses.Num() == PMCInstances.Num();

	// Check if it is safe to draw all new instances
	if (TimelineIndex + NumNewInstances < TimelinePoses.Num())
	{
		// Safe to draw all instances
		while (NumNewInstances > 0)
		{
			if (TimelineIndex < TimelineMaxNumInstances)
			{
				bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);
			}
			else
			{
				PMCInstances[TimelineIndex-TimelineMaxNumInstances]->SetVisibility(false);
				bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);
			}
			TimelineIndex++;
			NumNewInstances--;
		}
	}
	else
	{
		// Reached end of the poses array, add only remaining values
		while (TimelinePoses.IsValidIndex(TimelineIndex))
		{
			if (TimelineIndex < TimelineMaxNumInstances)
			{
				bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);				bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);
			}
			else
			{
				PMCInstances[TimelineIndex - TimelineMaxNumInstances]->SetVisibility(false);
				bInstancesAlreadyCreated ? PMCInstances[TimelineIndex]->SetVisibility(true) : AddInstance(TimelinePoses[TimelineIndex]);
			}
			TimelineIndex++;
		}

		// Hide existing instances if timeline should be looped
		if (bLoopTimeline)
		{
			// Avoid destroying the instances
			HideInstances();
			// TimelinePoses.Empty(); commented out since it is used to detect if the instances were created, or create explicit flag
			TimelineIndex = 0;
		}
		else
		{
			ClearAndStopTimeline();
		}
	}
}

//   Set visual without the materials (avoid boilerplate code)
void USLVizSkeletalMeshMarker::SetPoseableMeshComponentVisual(USkeletalMesh* SkelMesh)
{
	// Clear any previous data
	Reset();
	
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		PMCRef = NewObject<UPoseableMeshComponent>(this);
		PMCRef->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PMCRef->bPerBoneMotionBlur = false;
		PMCRef->bHasMotionBlurVelocityMeshes = false;
		PMCRef->bSelectable = false;
		PMCRef->SetVisibility(false);
		//PMCRef->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		PMCRef->RegisterComponent();
	}

	// Set the reference visual
	PMCRef->SetSkeletalMesh(SkelMesh);
}

// Create poseable mesh component instance attached and registered to this marker
UPoseableMeshComponent* USLVizSkeletalMeshMarker::CreateNewPoseableMeshInstance()
{
	UPoseableMeshComponent* NewPMC = DuplicateObject<UPoseableMeshComponent>(PMCRef, this);
	NewPMC->SetVisibility(true);
	//NewPMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	NewPMC->RegisterComponent();
	return NewPMC;
}
/* End VizMarker interface */
