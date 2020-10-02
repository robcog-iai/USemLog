// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Markers/SLVizBaseMarker.h"
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

	// Called every frame, used for timeline visualizations, activated and deactivated on request
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	// Add instances at pose with bones as optional parameter
	void AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses = TMap<int32, FTransform>());

	// Add instance with bone poses
	void AddInstance(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose);

	// Add instances with the poses with bones as optinal parameters
	void AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray = TArray<TMap<int32, FTransform>>());

	// Add instances with bone poses
	void AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses);

	// Add instances with timeline update
	void AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses, float Duration, bool bLoop, float UpdateRate = -1.f);

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

	// Set instances visibility to false
	void HideInstances();

	// Clear the timeline and the related members
	void ClearTimelineData();

private:
	// Set visual without the materials (avoid boilerplate code)
	void SetPoseableMeshComponentVisual(USkeletalMesh* SkelMesh);

	// Create poseable mesh component instance attached and registered to this marker
	UPoseableMeshComponent* CreateNewPoseableMeshInstance();

protected:
	// Poseable mesh reference
	UPROPERTY(VisibleAnywhere)
	UPoseableMeshComponent* PMCRef;

	// Skeletal instances
	UPROPERTY(VisibleAnywhere)
	TArray<UPoseableMeshComponent*> PMCInstances;

	// Timeline poses
	TArray<TPair<FTransform, TMap<int32, FTransform>>> TimelinePoses;

	// Timeline position in the array
	int32 TimelineIndex;

	// Flag to loop the timeline
	bool bLoopTimeline;

	// Duration in which the timeline should be drawed
	float TimelineDuration;
};
