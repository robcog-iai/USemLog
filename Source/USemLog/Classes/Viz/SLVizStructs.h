// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
	ArrowX			UMETA(DisplayName = "ArrowX"),
	ArrowY			UMETA(DisplayName = "ArrowY"),
	ArrowZ			UMETA(DisplayName = "ArrowZ"),
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
struct FSLVizEpisodePlayParams
{
	GENERATED_BODY()

	// For negative value it will start from first frame
	UPROPERTY(EditAnywhere, Category = "Time")
	float StartTime = -1.f;

	// For negative value it will run until the last frame
	UPROPERTY(EditAnywhere, Category = "Time")
	float EndTime = -1.f;

	// Repeat replay after finishing
	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bLoop = false;

	// How quickly to move to the next frame (if negative, it will calculate an average update rate from the episode data)
	UPROPERTY(EditAnywhere, Category = "Properties")
	float UpdateRate = -1.f;

	// How many steps to update every frame
	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 StepSize = 1;

	// Default ctor
	FSLVizEpisodePlayParams() {};

	// Init ctor
	FSLVizEpisodePlayParams(bool bLoopValue, float InUpdateRate, int32 InStepSize, const FString& InTargetViewId) :
	bLoop(bLoopValue),
	UpdateRate(InUpdateRate),
	StepSize(InStepSize) {}
};

/**
 * Parameters for timeline markers
 */
USTRUCT()
struct FSLVizTimelineParams
{
	GENERATED_BODY()

	// For negative value the duration will be computed
	UPROPERTY(EditAnywhere, Category = "Time")
	float Duration = -1.f;

	// For negative value it will run every tick
	UPROPERTY(EditAnywhere, Category = "Time")
	float UpdateRate = -1.f;

	// Maximum number of instaces to simultaneusly draw (negative values ignored)
	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 MaxNumInstances = INDEX_NONE;

	// Repeat timeline after finishing
	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bLoop = false;
};

