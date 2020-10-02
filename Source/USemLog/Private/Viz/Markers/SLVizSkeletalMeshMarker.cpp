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

	int32 NumInstancesToDraw = (DeltaTime * TimelinePoses.Num()) / TimelineDuration;
	if (TimelineIndex + NumInstancesToDraw < TimelinePoses.Num())
	{
		// It is safe to add all instances
		while (NumInstancesToDraw > 0)
		{
			if (!PMCInstances.IsValidIndex(TimelineIndex))
			{
				AddInstance(TimelinePoses[TimelineIndex]); // Create new instance
			}
			else
			{
				PMCInstances[TimelineIndex]->SetVisibility(true); // Instance is already there, set it to visible
			}
			TimelineIndex++;
			NumInstancesToDraw--;
		}
	}
	else
	{
		// Reached end of the poses array, add remaining values
		while (TimelinePoses.IsValidIndex(TimelineIndex))
		{
			if (!PMCInstances.IsValidIndex(TimelineIndex))
			{
				AddInstance(TimelinePoses[TimelineIndex]); // Create new instance
			}
			else
			{
				PMCInstances[TimelineIndex]->SetVisibility(true); // Instance is already there, set it to visible
			}
			TimelineIndex++;
		}

		// Check if the timeline should be repeated or stopped
		if (bLoopTimeline)
		{
			HideInstances(); // Hide all instances
			TimelinePoses.Empty(); // Cached poses can be cleared since we are re-iterating the existing instances
			TimelineIndex = 0;
		}
		else
		{
			ClearTimelineData();
		}
	}
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

// Set the visual properties of the skeletal mesh, visualize only selected material index (use original materials)
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMesh* SkelMesh, int32 MaterialIndex)
{
	// Create the poseable mesh reference and the dynamic material
	SetPoseableMeshComponentVisual(SkelMesh);

	// Hide unwanted material slots
	for (int32 MatIdx = 0; MatIdx < PMCRef->GetNumMaterials(); ++MatIdx)
	{
		if (MatIdx != MaterialIndex)
		{
			PMCRef->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible);
		}
	}
}

// Set the visual properties of the skeletal mesh, visualize only selected material index
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMesh* SkelMesh, int32 MaterialIndex,
	const FLinearColor& InColor, ESLVizMaterialType InMaterialType)
{
	// Create the poseable mesh reference and the dynamic material
	SetPoseableMeshComponentVisual(SkelMesh);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < PMCRef->GetNumMaterials(); ++MatIdx)
	{
		MatIdx != MaterialIndex ? PMCRef->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible)
			: PMCRef->SetMaterial(MatIdx, DynamicMaterial);
	}
}
// Visualize only selected material indexes (use original materials)
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMesh* SkelMesh, const TArray<int32>& MaterialIndexes)
{
	// Create the poseable mesh reference and the dynamic material
	SetPoseableMeshComponentVisual(SkelMesh);

	// Hide unwanted material slots
	for (int32 MatIdx = 0; MatIdx < PMCRef->GetNumMaterials(); ++MatIdx)
	{
		if (!MaterialIndexes.Contains(MatIdx))
		{
			PMCRef->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible);
		}
	}
}

// Visualize only selected material indexes
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMesh* SkelMesh, const TArray<int32>& MaterialIndexes,
	const FLinearColor& InColor, ESLVizMaterialType InMaterialType)
{
	// Create the poseable mesh reference and the dynamic material
	SetPoseableMeshComponentVisual(SkelMesh);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < PMCRef->GetNumMaterials(); ++MatIdx)
	{
		 !MaterialIndexes.Contains(MatIdx) ? PMCRef->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible)
			: PMCRef->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Add instances at pose
void USLVizSkeletalMeshMarker::AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses)
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
	PMC->SetWorldTransform(Pose);

	for (const auto& BonePosePair : BonePoses)
	{	
		const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
		PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
	}

	PMCInstances.Add(PMC);
}

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

	for (const auto& BonePosePair : SkeletalPose.Value)
	{
		const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
		PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
	}

	PMCInstances.Add(PMC);
}

// Add instances with the poses
void USLVizSkeletalMeshMarker::AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray)
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Make sure the size of the two arrays are the same
	const bool bIncludeBones = Poses.Num() == BonePosesArray.Num();
	
	int32 PoseIdx = 0;
	for (const auto& P : Poses)
	{
		UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
		PMC->SetWorldTransform(P);

		if (bIncludeBones)
		{
			for (const auto& BonePosePair : BonePosesArray[PoseIdx])
			{
				const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
				PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
			}
			PoseIdx++;
		}

		PMCInstances.Add(PMC);
	}
}

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

		for (const auto& BonePosePair : SkelPosePair.Value)
		{
			const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
			PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
		}

		PMCInstances.Add(PMC);
	}
}

// Add instances with timeline update
void USLVizSkeletalMeshMarker::AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses, float Duration, bool bLoop, float UpdateRate)
{
	if (!PMCRef || !PMCRef->IsValidLowLevel() || PMCRef->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (Duration <= 0.008f)
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
	TimelineDuration = Duration;
	bLoopTimeline = bLoop;
	TimelineIndex = 0;

	// Start timeline
	if (UpdateRate > 0.f)
	{
		SetComponentTickInterval(UpdateRate);
	}
	SetComponentTickEnabled(true);
}

// Unregister the component, remove it from its outer Actor's Components array and mark for pending kill
void USLVizSkeletalMeshMarker::DestroyComponent(bool bPromoteChildren)
{
	for (const auto& PMCInst : PMCInstances)
	{
		if (PMCInst && PMCInst->IsValidLowLevel() && PMCInst->IsPendingKillOrUnreachable())
		{
			PMCInst->DestroyComponent();
		}
	}

	if (PMCRef && PMCRef->IsValidLowLevel() && PMCRef->IsPendingKillOrUnreachable())
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

// Set instances visibility to false
void USLVizSkeletalMeshMarker::HideInstances()
{
	for (const auto& PMC : PMCInstances)
	{
		PMC->SetVisibility(false);
	}
}

// Clear the timeline and the related members
void USLVizSkeletalMeshMarker::ClearTimelineData()
{
	if (IsComponentTickEnabled())
	{
		SetComponentTickEnabled(false);
		SetComponentTickInterval(-1.f); // Tick every frame by default (If less than or equal to 0 then it will tick every frame)
	}
	TimelineIndex = INDEX_NONE;
	TimelinePoses.Empty();
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
	// TODO double check that this works
	UPoseableMeshComponent* NewPMC = DuplicateObject<UPoseableMeshComponent>(PMCRef, this);
	//NewPMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	NewPMC->SetVisibility(true);
	return NewPMC;
}
/* End VizMarker interface */
