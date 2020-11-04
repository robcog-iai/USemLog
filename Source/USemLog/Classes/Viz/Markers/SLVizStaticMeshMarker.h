// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Markers/SLVizBaseMarker.h"
#include "Viz/SLVizStructs.h"
#include "SLVizStaticMeshMarker.generated.h"

// Forward declarations
class UInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * Class capable of visualizing multiple types of markers as instanced static meshes
 */
UCLASS()
class USEMLOG_API USLVizStaticMeshMarker : public USLVizBaseMarker
{
	GENERATED_BODY()

public:
	// Constructor
	USLVizStaticMeshMarker();

	// Called every frame, used for timeline visualizations, activated and deactivated on request
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Set the visual properties of the instanced mesh using the mesh original materials
	void SetVisual(UStaticMesh* SM);

	// Set the visual properties of the instanced mesh
	void SetVisual(UStaticMesh* SM,	const FLinearColor& InColor, ESLVizMaterialType InMaterialType = ESLVizMaterialType::Unlit);

	// Update the visual mesh type
	void UpdateStaticMesh(UStaticMesh* SM);

	// Add instances at pose
	void AddInstance(const FTransform& Pose);

	// Add instances with the poses
	void AddInstances(const TArray<FTransform>& Poses);

	// Add instances with timeline update
	void AddInstances(const TArray<FTransform>& Poses, const FSLVizTimelineParams& TimelineParams);

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

	// Virtual add instance function
	virtual void AddInstanceChecked(const FTransform& Pose);

	// Virtual update instance transform
	virtual bool UpdateInstanceTransform(int32 Index, const FTransform& Pose);

	// Virtual add instances function
	virtual void AddInstancesChecked(const TArray<FTransform>& Poses);

	// Clear the timeline and the related members
	void ClearAndStopTimeline();

	// Update timeline with the given number of new instances
	void UpdateTimeline(int32 NumNewInstances);

	// Update timeline with max number of instances
	void UpdateTimelineWithMaxNumInstances(int32 NumNewInstances);

protected:
	// A component that efficiently renders multiple instances of the same StaticMesh.
	UPROPERTY()
	UInstancedStaticMeshComponent* ISMC;

	// Timeline poses
	TArray<FTransform> TimelinePoses;
};
