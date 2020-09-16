// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Marker/SLVizBaseMarker.h"
#include "SLVizSkeletalMeshMarker.generated.h"

// Forward declarations
class USkeletalMesh;
class UPoseableMeshComponent;

/**
 * Class capable of visualizing skeletal meshes as arrays of poseable meshes
 */
UCLASS()
class USEMLOG_API USLVizSkeletalMeshMarker : public USLVizBaseMarker
{
	GENERATED_BODY()

public:
	// Constructor
	USLVizSkeletalMeshMarker();

	// Set the visual properties of the skeletal mesh (use original materials)
	void SetVisual(USkeletalMesh* SkelMesh);

	// Set the visual properties of the skeletal mesh
	void SetVisual(USkeletalMesh* SkelMesh,	const FLinearColor& InColor,
		ESLVizMaterialType InMaterialType = ESLVizMaterialType::Unlit);

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

	// Add instances at pose
	void AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses = TMap<int32, FTransform>());

	// Add instances with the poses
	void AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray = TArray<TMap<int32, FTransform>>());

	//~ Begin ActorComponent Interface
	// Unregister the component, remove it from its outer Actor's Components array and mark for pending kill
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	//~ End ActorComponent Interface

	/* Begin VizMarker interface */
	// Reset visuals and poses
	virtual void Reset() override;

protected:
	// Reset visual related data
	virtual void ResetVisuals() override;

	// Reset instances (poses of the visuals)
	virtual void ResetPoses() override;
	/* End VizMarker interface */

private:
	// Set visual without the materials (avoid boilerplate code)
	void SetPoseableMeshComponentVisual(USkeletalMesh* SkelMesh);

	// Create poseable mesh component instance attached and registered to this marker
	UPoseableMeshComponent* CreateNewPoseableMeshInstance();

protected:
	// Poseable mesh reference
	UPROPERTY()
	UPoseableMeshComponent* PMCRef;

	// Skeletal instances
	UPROPERTY()
	TArray<UPoseableMeshComponent*> PMCInstances;
};
