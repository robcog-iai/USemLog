// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVUtils.h"

// Create new image with the pixels replaced 
TArray<FColor> FSLCVUtils::ReplacePixels(const TArray<FColor>& InBitmap, FColor FromColor, FColor ToColor, float Tolerance)
{	
	// Make a copy of the image
	TArray<FColor> NewImage = InBitmap;

	// Iterate image and switch colors
	if (Tolerance > 0)
	{
		for (auto& Pixel : NewImage)
		{
			if (FSLCVUtils::ManhattanDistance(Pixel, FromColor) < Tolerance)
			{
				Pixel = ToColor;
			}
		}
	}
	else
	{
		for (auto& Pixel : NewImage)
		{
			if (Pixel == FromColor)
			{
				Pixel = ToColor;
			}
		}
	}

	return NewImage;
}
