// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLVisionStructs.h"

/**
* Image pixel color related data, convenient mapping of mask colors to their semantic data
*/
struct FSLVisionEntityInfo
{
	// Default ctor
	FSLVisionEntityInfo() {};
	
	// Init ctor
	FSLVisionEntityInfo(const FString& InClass, const FString& InId, const FString& InOrigMaskColor) :
		Class(InClass), Id(InId), OrigMaskColor(InOrigMaskColor) {};

	// Class name
	FString Class;

	// Id
	FString Id;

	// Original mask color (used for direct access for restoring the mask image)
	FString OrigMaskColor;

};

/**
* Image pixel color related data, convenient mapping of mask colors to their semantic data
*/
struct FSLVisionSkelInfo
{
	// Default ctor
	FSLVisionSkelInfo() {};

	// Init ctor
	FSLVisionSkelInfo(const FString& InClass, const FString& InId, const FString& InBoneClass, const FString& InOrigMaskColor) :
		Class(InClass), Id(InId), BoneClass(InBoneClass), OrigMaskColor(InOrigMaskColor) {};

	// Skeletal entity class name
	FString Class;

	// Id
	FString Id;

	// Bone class
	FString BoneClass;

	// Original mask color (used for direct access for restoring the mask image)
	FString OrigMaskColor;
};

/**
* Image pixel color related data, eventually the pixel color will be mapped to its semantic values (class, id..)
*/
struct FSLVisionImageColorInfo
{
	// Default constructor
	FSLVisionImageColorInfo() : Num(0) {};

	// Init ctor
	FSLVisionImageColorInfo(int32 InNum, const FIntPoint& InMinBB, const FIntPoint& InMaxBB) : Num(InNum), MinBB(InMinBB), MaxBB(InMaxBB) {};

	// Number of pixels in image
	int32 Num;

	// Min bounding box value in image
	FIntPoint MinBB;

	// Max bounding box value in image
	FIntPoint MaxBB;

	// Cache the original mask color represented by the pixel
	FColor OriginalMaskColor;
};

/**
 * 
 */
class FSLVisionImageHandler
{
public:
	// Ctor
	FSLVisionImageHandler();

	// Load the color to entities mapping
	bool Init();

	// Clear init flag and mappings
	void Reset();

	//// Get entities from mask image
	//void GetEntities(const TArray<FColor>& InMaskBitmap, int32 ImgWidth, int32 ImgHeight, FSLVisionViewData& OutViewData) const;

	// Restore image (the screenshot image pixel colors are a bit offseted from the supposed mask value) and get the entities from mask image
	void GetDataAndRestoreImage(TArray<FColor>& MaskBitmap, int32 ImgWidth, int32 ImgHeight, FSLVisionViewData& OutViewData) const;

private:
	/* Helper functions */
	// Restore the color of the pixel to its original mask value (offseted by screenshot rendering artifacts), returns true if restoration happened
	bool RestoreColorValueFromArray(FColor& PixelColor, const TArray<FColor>& InOriginalMaskColors, uint8 Tolerance = 13) const;

	// Check if the two colors are equal with a tolerance
	FORCEINLINE static bool AlmostEqual(const FColor& C1, const FColor& C2, uint8 Tolerance = 0)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B) <= Tolerance;
	}

private:
	// Init flag
	bool bIsInit;

	// Rendered color to entity data
	TMap<FColor, FSLVisionEntityInfo> RenderedColorToEntityInfo;

	// Rendered color to skeletal entity data
	TMap<FColor, FSLVisionSkelInfo> RenderedColorToSkelInfo;

	//// Used to check the mapping from the rendered color to the original one
	//TMap<FColor, FColor> RenderedToOriginal;

	//// Used for quick checks for offseted colors actual value (TArray is used instead of TSet, because it has FindByPredicate functionality)
	//TArray<FColor> OriginalMaskColors;

	///* Constants */
	//// Mask color equality tolerance
	//constexpr static uint8 ColorDiffTolerance = 13;
};
