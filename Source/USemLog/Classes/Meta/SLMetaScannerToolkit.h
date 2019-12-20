// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
 * Helper class for the image 
 */
class FSLMetaScannerToolkit
{
public:
	// Ctor
	FSLMetaScannerToolkit();

	// Get the bounding box and the number of pixels the item occupies in the image
	void GetItemPixelNumAndBB(const TArray<FColor>& InBitmap, int32 Width, int32 Height, int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax);

	// Get the bounding box and the number of pixels the color occupies in the image
	void GetColorPixelNumAndBB(const TArray<FColor>& InBitmap, const FColor& Color, int32 Width, int32 Height, int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax);

	// Get the number of pixels that the item occupies in the image
	int32 GetItemPixelNum(const TArray<FColor>& Bitmap);

	// Get the number of pixels of the given color in the image
	int32 GetColorPixelNum(const TArray<FColor>& Bitmap, const FColor& Color) const;

	// Count and check validity of the number of pixels the item represents in the image;
	void CountItemPixelNumWithCheck(const TArray<FColor>& Bitmap, int32 ResX, int32 ResY);

	// Get the number of pixels of the given two colors in the image
	void GetColorsPixelNum(const TArray<FColor>& Bitmap, const FColor& ColorA, int32& OutNumA, const FColor& ColorB, int32& OutNumB);

	// Generate sphere scan poses
	void GenerateSphereScanPoses(uint32 MaxNumOfPoints, float Radius, TArray<FTransform>& OutTransforms);

private:
	/* Constants */
	// Mask color
	constexpr static const FColor WhiteMaskColorConst = FColor(255, 255, 255);
};
