// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Marker/SLVizBaseMarker.h"
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

	// Add instances with the poses as a timeline
	void AddTimeline(const TArray<FTransform>& Poses, float UpdateRate,  bool bLoop, float StartDelay = -1.f);

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

	// Used as an timer update callback when the instance should be added as a timeline
	void TimelineUpdateCallback();

	// Clear the timeline and the related members
	void ResetTimeline();

protected:
	// A component that efficiently renders multiple instances of the same StaticMesh.
	UPROPERTY()
	UInstancedStaticMeshComponent* ISMC;

	// Timer handle used for updating the timeline
	FTimerHandle TimelineTimerHandle;

	// Timeline poses
	TArray<FTransform> TimelinePoses;

	// Timeline position in the array
	int32 TimelineIndex;

	// Flag to loop the timeline
	bool bLoopTimeline;
};
