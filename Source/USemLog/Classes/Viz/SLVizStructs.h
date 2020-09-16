// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLVizStructs.generated.h"

/**
 * Dynamic material types
 */
UENUM()
enum class ESLVizMaterialType : uint8
{
	NONE				UMETA(DisplayName = "NONE"),
	Lit					UMETA(DisplayName = "Lit"),
	Unlit				UMETA(DisplayName = "Unlit"),
	Additive			UMETA(DisplayName = "Additive"),
	Translucent			UMETA(DisplayName = "Translucent")
};

/**
 * Viz visual parameters (color and material type)
 */
USTRUCT()
struct FSLVizVisualParams
{
	GENERATED_BODY();

	// Color to apply to the dynamic material
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	// Type of dynamic material
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;

	// Material slots to which to apply the visual parameters (if empty, apply to all slots)
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TArray<int32> MaterialSlots;

	// Default ctor
	FSLVizVisualParams() {};

	// Init ctor
	FSLVizVisualParams(const FLinearColor& InColor, ESLVizMaterialType InMaterialType, const TArray<int32>& InMaterialSlots)
	: Color(InColor), MaterialType(InMaterialType), MaterialSlots(InMaterialSlots) {};

	// Init ctor
	FSLVizVisualParams(const FLinearColor& InColor, ESLVizMaterialType InMaterialType, int32 InMaterialSlot)
		: Color(InColor), MaterialType(InMaterialType), MaterialSlots(TArray<int32>{InMaterialSlot}) {};

	// Init ctor
	FSLVizVisualParams(const FLinearColor& InColor, ESLVizMaterialType InMaterialType)
		: Color(InColor), MaterialType(InMaterialType) {};

	// Init ctor
	FSLVizVisualParams(const FLinearColor& InColor)
		: Color(InColor) {};

	// Init ctor
	FSLVizVisualParams(const FLinearColor& InColor, const TArray<int32>& InMaterialSlots)
		: Color(InColor), MaterialSlots(InMaterialSlots) {};

	// Init ctor
	FSLVizVisualParams(const FLinearColor& InColor, int32 InMaterialSlot)
		: Color(InColor), MaterialSlots(TArray<int32>{InMaterialSlot}) {};
};

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
 * Data about the currently highlighted individuals (mesh, material index)
 */
USTRUCT()
struct FSLVizIndividualHighlightData
{
	GENERATED_BODY();

	// The mesh that is currently highlighted
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	UMeshComponent* MeshComponent;

	// The mesh slots that are highlighted (empty means all)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<int32> MaterialSlots;

	// Default ctor
	FSLVizIndividualHighlightData() {};

	// Init ctor
	FSLVizIndividualHighlightData(UMeshComponent* InMeshComponent) : MeshComponent(InMeshComponent) {};

	// Init ctor
	FSLVizIndividualHighlightData(UMeshComponent* InMeshComponent, int32 InMaterialSlot)
		: MeshComponent(InMeshComponent), MaterialSlots(TArray<int32>{InMaterialSlot}) {};

	// Init ctor
	FSLVizIndividualHighlightData(UMeshComponent* InMeshComponent, const TArray<int32>& InMaterialSlots)
		: MeshComponent(InMeshComponent), MaterialSlots(InMaterialSlots) {};
};

