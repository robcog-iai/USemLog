// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class USEMLOG_API FSLCVUtils
{
public:
	// Create new image with the pixels replaced 
	static TArray<FColor> ReplacePixels(const TArray<FColor>& InBitmap, FColor FromColor, FColor ToColor, float Tolerance = 0);

	// Get the manhattan distance between the two colors
	FORCEINLINE static int32 ManhattanDistance(const FColor& C1, const FColor& C2)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B);
	}
};
