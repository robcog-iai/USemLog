// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Marker/SLVizSkeletalMeshMarker.h"
#include "Viz/SLVizAssets.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizSkeletalMeshMarker::USLVizSkeletalMeshMarker()
{
	PrimaryComponentTick.bCanEverTick = false;
	PMCReference = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("SLViz_SkelMarker_Reference"));
	PMCReference->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PMCReference->bPerBoneMotionBlur = false;
	PMCReference->bHasMotionBlurVelocityMeshes = false;
	PMCReference->SetVisibility(false);
}

// Set the visual properties of the skeletal mesh
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMeshComponent* SkelMC,
	const FLinearColor& InColor, ESLVizMarkerMaterialType InMaterialType)
{
	// Clear any previous data
	Reset();

	// Set the reference visual
	PMCReference->SetSkeletalMesh(SkelMC->SkeletalMesh);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < PMCReference->GetNumMaterials(); ++MatIdx)
	{
		PMCReference->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Set the visual properties of the skeletal mesh, visualize only selected material index
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMeshComponent* SkelMC, int32 MaterialIndex,
	const FLinearColor& InColor, ESLVizMarkerMaterialType InMaterialType)
{
	// Clear any previous data
	Reset();

	// Set the reference visual
	PMCReference->SetSkeletalMesh(SkelMC->SkeletalMesh);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < SkelMC->GetNumMaterials(); ++MatIdx)
	{
		MatIdx != MaterialIndex ? PMCReference->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible)
			: PMCReference->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Visualize only selected material indexes
void USLVizSkeletalMeshMarker::SetVisual(USkeletalMeshComponent* SkelMC, TArray<int32>& MaterialIndexes,
	const FLinearColor& InColor, ESLVizMarkerMaterialType InMaterialType)
{
	// Clear any previous data
	Reset();

	// Set the reference visual
	PMCReference->SetSkeletalMesh(SkelMC->SkeletalMesh);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < SkelMC->GetNumMaterials(); ++MatIdx)
	{
		 !MaterialIndexes.Contains(MatIdx) ? PMCReference->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible)
			: PMCReference->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Add instances at pose
void USLVizSkeletalMeshMarker::AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses)
{
	UPoseableMeshComponent* PMC = CreateNewPoseableMeshInstance();
	PMC->SetWorldTransform(Pose);

	for (const auto& BonePosePair : BonePoses)
	{	
		const FName BoneName = PMC->GetBoneName(BonePosePair.Key);
		PMC->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
	}

	PMCInstances.Add(PMC);
}

// Add instances with the poses
void USLVizSkeletalMeshMarker::AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray)
{
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
	PMCReference->EmptyOverrideMaterials();
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

// Create poseable mesh component instance attached and registered to this marker
UPoseableMeshComponent* USLVizSkeletalMeshMarker::CreateNewPoseableMeshInstance()
{
	UPoseableMeshComponent* NewPMC = DuplicateObject<UPoseableMeshComponent>(PMCReference, this);
	NewPMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	NewPMC->SetVisibility(true);
	return NewPMC;
}
/* End VizMarker interface */
