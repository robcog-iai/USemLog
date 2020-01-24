// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisionOverlapCalc.generated.h"

/**
 * Calculates overlap percentages for entities in an image
 */
UCLASS()
class USLVisionOverlapCalc : public UObject
{
	GENERATED_BODY()

protected:
	// Trigger the screenshot on the game thread
	void RequestScreenshot();

	// Called when the screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);
};
