// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Marker/SLVizBaseMarker.h"
#include "SLVizSkeletalMeshMarker.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UPoseableMeshComponent;

/**
 * Class capable of visualizing multiple types of markers as instanced static meshes
 */
UCLASS()
class USEMLOG_API USLVizSkeletalMeshMarker : public USLVizBaseMarker
{
	GENERATED_BODY()

public:
	// Constructor
	USLVizSkeletalMeshMarker();

	// Set the visual properties of the skeletal mesh
	void SetVisual(USkeletalMeshComponent* SkelMC,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMarkerMaterialType InMaterialType = ESLVizMarkerMaterialType::Unlit);

	// Set the visual properties of the skeletal mesh, visualize only selected material index
	void SetVisual(USkeletalMeshComponent* SkelMC, int32 MaterialIndex,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMarkerMaterialType InMaterialType = ESLVizMarkerMaterialType::Unlit);

	// Visualize only selected material indexes
	void SetVisual(USkeletalMeshComponent* SkelMC, TArray<int32>& MaterialIndexes,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMarkerMaterialType InMaterialType = ESLVizMarkerMaterialType::Unlit);

	// Add instances at pose
	void AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses = TMap<int32, FTransform>());

	// Add instances with the poses
	void AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray = TArray<TMap<int32, FTransform>>());

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
	// Create poseable mesh component instance attached and registered to this marker
	UPoseableMeshComponent* CreateNewPoseableMeshInstance();

protected:
	// Poseable mesh reference
	//UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	UPoseableMeshComponent* PMCReference;

	// Skeletal instances
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<UPoseableMeshComponent*> PMCInstances;
};
