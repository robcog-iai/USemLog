// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Markers/SLVizSkeletalMeshMarker.h"
#include "SLVizSkeletalBoneMeshMarker.generated.h"

/**
 * Class capable of visualizing skeletal meshe bones as arrays of poseable meshes
 */
UCLASS()
class USEMLOG_API USLVizSkeletalBoneMeshMarker : public USLVizSkeletalMeshMarker
{
	GENERATED_BODY()

public:
	// Constructor
	USLVizSkeletalBoneMeshMarker();

	// Set the visual properties of the skeletal mesh, visualize only selected material index (use original materials)
	void SetVisual(USkeletalMesh* SkelMesh, int32 MaterialIndex);

	// Set the visual properties of the skeletal mesh, visualize only selected material index
	void SetVisual(USkeletalMesh* SkelMesh, int32 MaterialIndex, const FLinearColor& InColor,
		ESLVizMaterialType InMaterialType = ESLVizMaterialType::Unlit);

	// Visualize only selected material indexes (use original materials)
	void SetVisual(USkeletalMesh* SkelMesh, const TArray<int32>& MaterialIndexes);

	// Visualize only selected material indexes
	void SetVisual(USkeletalMesh* SkelMesh, const TArray<int32>& MaterialIndexes, const FLinearColor& InColor,
		ESLVizMaterialType InMaterialType = ESLVizMaterialType::Unlit);
};
