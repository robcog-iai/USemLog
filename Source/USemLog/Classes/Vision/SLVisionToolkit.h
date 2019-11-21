// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"

/**
 * Helper functions for the vision logger
 */
class FSLVisionToolkit
{
public:
	// Tag non-movable objects as static
	static void VisionToolkitExample(UWorld* World, bool bOverwriteProperties);
	
private:
	// Randomly generate unique visual masks
	static void RandomlyGenerateVisualMasks(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance);

	/* Helpers */
	// Get the distance between the colors
	FORCEINLINE static float ColorDistance(const FColor& C1, const FColor C2)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B);
	}

	// Check if the two colors are equal with a tolerance
	FORCEINLINE static float ColorEqual(const FColor& C1, const FColor C2, int32 Tolerance)
	{
		if(Tolerance < 1)
		{
			return C1 == C2;
		}
		else
		{
			return ColorDistance(C1, C2) <= Tolerance;
		}
	}

	// Make a random rgb color
	FORCEINLINE static FColor ColorRandomRGB()
	{
		return FColor((uint8)(FMath::FRand()*255.f), (uint8)(FMath::FRand()*255.f), (uint8)(FMath::FRand()*255.f));
	}
};

