// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Marker/SLVizBaseMarker.h"
#include "SLVizStaticMeshMarker.generated.h"

// Forward declarations
class UStaticMeshComponent;
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

	// Set the visual properties of the instanced mesh
	void SetVisual(UStaticMeshComponent* SMC,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMarkerMaterialType InMaterialType = ESLVizMarkerMaterialType::Unlit);

	// Update the visual mesh type
	void UpdateStaticMesh(UStaticMeshComponent* SMC);

	// Add instances at pose
	void AddInstance(const FTransform& Pose);

	// Add instances with the poses
	void AddInstances(const TArray<FTransform>& Poses);

	/* Begin VizMarker interface */
	// Reset visuals and poses
	virtual void Reset() override;

protected:
	// Reset visual related data
	virtual void ResetVisuals() override;

	// Reset instances (poses of the visuals)
	virtual void ResetPoses() override;
	/* End VizMarker interface */

protected:
	// A component that efficiently renders multiple instances of the same StaticMesh.
	UInstancedStaticMeshComponent* ISMComponent;
};
