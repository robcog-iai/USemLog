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
	PMCRef = nullptr;
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
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
	//NewPMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	NewPMC->SetVisibility(true);
	return NewPMC;
}
/* End VizMarker interface */
