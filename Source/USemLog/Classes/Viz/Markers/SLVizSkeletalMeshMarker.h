// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Markers/SLVizBaseMarker.h"
#include "Viz/SLVizStructs.h"
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
	void SetVisual(USkeletalMesh* SkelMesh,	const FLinearColor& InColor, ESLVizMaterialType InMaterialType = ESLVizMaterialType::Unlit);

	//// Add instances at pose with bones as optional parameter
	//void AddInstance(const FTransform& Pose, const TMap<int32, FTransform>& BonePoses = TMap<int32, FTransform>());

	// Add instance with bone poses
	void AddInstance(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose);

	//// Add instances with the poses with bones as optional parameters
	//void AddInstances(const TArray<FTransform>& Poses, const TArray<TMap<int32, FTransform>>& BonePosesArray = TArray<TMap<int32, FTransform>>());

	// Add instances with bone poses
	void AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses);

	// Add instances with timeline update
	void AddInstances(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		const FSLVizTimelineParams& TimelineParams);

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

	//// Update intial timeline iteration (create the instances)
	//void UpdateInitialTimeline(float DeltaTime);

	//// Update loop timeline (set instaces visibility)
	//void UpdateLoopTimeline(float DeltaTime);

	// Set instances visibility to false
	void HideInstances();

	// Clear the timeline and the related members
	void ClearAndStopTimeline();

	// Update timeline with the given number of new instances
	void UpdateTimeline(int32 NumNewInstances);

	// Update timeline with max number of instances
	void UpdateTimelineWithMaxNumInstances(int32 NumNewInstances);

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

	// Timeline poses
	TArray<TPair<FTransform, TMap<int32, FTransform>>> TimelinePoses;
};
