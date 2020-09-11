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
 * Viz parameters
 */
USTRUCT()
struct FSLVizVisualParams
{
	GENERATED_BODY();

	// Color to apply to the dynamic material
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	// Type of dynamic material
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;
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

