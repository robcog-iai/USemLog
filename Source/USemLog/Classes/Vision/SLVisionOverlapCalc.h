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

public:
	// Constructor
	USLVisionOverlapCalc();

	// Destructor
	~USLVisionOverlapCalc();

	// 
	void Init();

	// 
	void Start();

	// 
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Trigger the screenshot on the game thread
	void RequestScreenshot();

	// Called when the screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// Image filename if it is going to be saved locally (plus a visual msg from editor when the image is saved)
	FString CurrImageFilename;

	// Folder name where to store the images if they are going to be stored locally
	FString SaveLocallyFolderName;
};
