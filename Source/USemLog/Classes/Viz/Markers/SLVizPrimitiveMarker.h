// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Markers/SLVizStaticMeshMarker.h"
#include "SLVizPrimitiveMarker.generated.h"

// Forward declarations
class UInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * Class capable of visualizing multiple types of markers as instanced static meshes
 */
UCLASS()
class USEMLOG_API USLVizPrimitiveMarker : public USLVizStaticMeshMarker
{
	GENERATED_BODY()

public:
	// Constructor
	USLVizPrimitiveMarker();

	// Set the visual properties of the instanced mesh
	void SetVisual(ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box,
		float Size = .1f,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Update the primitive type
	void UpdatePrimitiveType(ESLVizPrimitiveMarkerType InType);

	// Update the visual scale property
	void UpdateSize(float Size);

	//// Add instances at pose (override since the scale needs to be set)
	//virtual void AddInstance(const FTransform& Pose) override;

	//// Add instances with the poses (override since the scale needs to be set)
	//virtual void AddInstances(const TArray<FTransform>& Poses) override;

protected:
	// Virtual add instance function
	virtual void AddInstanceChecked(const FTransform& Pose) override;

	// Virtual update instance transform
	virtual bool UpdateInstanceTransform(int32 Index, const FTransform& Pose) override;

	// Virtual add instances function
	virtual void AddInstancesChecked(const TArray<FTransform>& Poses) override;

	// Get the static mesh of the primitive type
	UStaticMesh* GetPrimitiveStaticMesh(ESLVizPrimitiveMarkerType InType) const;

protected:
	// Scale of the primitive mesh (mesh size is usually 1 meter)
	FVector MarkerScale;

	// Current visual type
	ESLVizPrimitiveMarkerType PrimitiveType;
};
