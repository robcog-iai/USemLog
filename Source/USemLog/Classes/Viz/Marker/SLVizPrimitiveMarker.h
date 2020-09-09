// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Viz/Marker/SLVizBaseMarker.h"
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
class USEMLOG_API USLVizPrimitiveMarker : public USLVizBaseMarker
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

private:
	// Get the static mesh of the primitive type
	UStaticMesh* GetPrimitiveStaticMesh(ESLVizPrimitiveMarkerType InType) const;

protected:
	// A component that efficiently renders multiple instances of the same StaticMesh.
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	UInstancedStaticMeshComponent* ISMComponent;

	// Scale of the primitive mesh (mesh size is usually 1 meter)
	FVector VisualScale;

	// Current visual type
	ESLVizPrimitiveMarkerType PrimitiveType;
};
