// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Markers/SLVizSkeletalBoneMeshMarker.h"
#include "Viz/SLVizAssets.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizSkeletalBoneMeshMarker::USLVizSkeletalBoneMeshMarker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PMCRef = nullptr;
}

// Set the visual properties of the skeletal mesh, visualize only selected material index (use original materials)
void USLVizSkeletalBoneMeshMarker::SetVisual(USkeletalMesh* SkelMesh, int32 MaterialIndex)
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
void USLVizSkeletalBoneMeshMarker::SetVisual(USkeletalMesh* SkelMesh, int32 MaterialIndex,
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
void USLVizSkeletalBoneMeshMarker::SetVisual(USkeletalMesh* SkelMesh, const TArray<int32>& MaterialIndexes)
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
void USLVizSkeletalBoneMeshMarker::SetVisual(USkeletalMesh* SkelMesh, const TArray<int32>& MaterialIndexes,
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
