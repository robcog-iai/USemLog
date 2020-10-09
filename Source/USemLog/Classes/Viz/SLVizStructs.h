// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLVizStructs.generated.h"

// Forward declarations
class UMeshComponent;

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
	UMeshComponent* MeshComponent = nullptr;

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


/**
 * A set of parameters to describe how to replay the episode data
 */
USTRUCT()
struct FSLVizEpisodeReplayPlayParams
{
	GENERATED_BODY()

	// Repeat replay after finishing
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLoop = false;

	// How quickly to move to the next frame (if negative, it will calculate an average update rate from the episode data)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate = -1.f;

	// How many steps to update every frame
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 StepSize = 1;

	// Default ctor
	FSLVizEpisodeReplayPlayParams() {};

	// Init ctor
	FSLVizEpisodeReplayPlayParams(bool bLoopValue, float InUpdateRate, int32 InStepSize, const FString& InTargetViewId) :
	bLoop(bLoopValue),
	UpdateRate(InUpdateRate),
	StepSize(InStepSize) {}
};