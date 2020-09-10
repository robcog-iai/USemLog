// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Marker/SLVizStaticMeshMarker.h"
#include "SLVizPrimitiveMarker.generated.h"

// Forward declarations
class UInstancedStaticMeshComponent;
class UStaticMesh;

/*
* Marker primitive types
*/
UENUM()
enum class ESLVizPrimitiveMarkerType : uint8
{
	NONE			UMETA(DisplayName = "NONE"),
	Box				UMETA(DisplayName = "Box"),
	Sphere			UMETA(DisplayName = "Sphere"),
	Cylinder		UMETA(DisplayName = "Cylinder"),
	Arrow			UMETA(DisplayName = "Arrow"),
	Axis			UMETA(DisplayName = "Axis")
};

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
		ESLVizMarkerMaterialType MaterialType = ESLVizMarkerMaterialType::Unlit);

	// Update the primitive type
	void UpdatePrimitiveType(ESLVizPrimitiveMarkerType InType);

	// Update the visual scale property
	void UpdateSize(float Size);

	// Add instances at pose (override since the scale needs to be set)
	virtual void AddInstance(const FTransform& Pose) override;

	// Add instances with the poses (override since the scale needs to be set)
	virtual void AddInstances(const TArray<FTransform>& Poses) override;

protected:
	// Get the static mesh of the primitive type
	UStaticMesh* GetPrimitiveStaticMesh(ESLVizPrimitiveMarkerType InType) const;

protected:
	// Scale of the primitive mesh (mesh size is usually 1 meter)
	FVector MarkerScale;

	// Current visual type
	ESLVizPrimitiveMarkerType PrimitiveType;
};
