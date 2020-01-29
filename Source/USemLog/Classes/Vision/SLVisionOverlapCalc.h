// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisionOverlapCalc.generated.h"

// Forward declaration
class UGameViewportClient;
class USLVisionLogger;

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

	// Give control to the overlap calc to pause and start its parent (vision logger)
	void Init(USLVisionLogger* InParent, FIntPoint InResolution);

	// Calculate overlaps for the givent scene
	void Start(struct FSLVisionViewData* CurrViewData);

	// Called when the scene is done, this un-pauses the parent as well
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

	// Overwrite previous resolution (typically smaller since the overlaps will be calculated in percentage)
	void InitScreenshotResolution(FIntPoint InResolution);

	// 
	bool SetupFirstItem();

	// 
	bool SetupNextItem();

private:
	// 
	bool SetupFirstEntity();

	// 
	bool SetupNextEntity();

	// 
	bool SetupFirstSkel();

	// 
	bool SetupNextSkel();

	// 
	void ApplyMaterial();

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// True if the skel entities are started
	bool bSkelArrayActive;

	// Use this to let the parent know that the overlap calc is done
	USLVisionLogger* Parent;

	// Pointer to the entities visible in the view
	TArray<FSLVisionViewEntityData>* Entities;

	// Pointer to the skeletal entities visible in the view
	TArray<FSLVisionViewSkelData>* SkelEntities;

	// Chached mask materials of the active instance
	TArray<UMaterialInterface*> CachedMaterials;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Delegate for the screenshot callback
	FDelegateHandle ScreenshotCallbackHandle;

	// Image filename if it is going to be saved locally (plus a visual msg from editor when the image is saved)
	FString CurrImageFilename;

	// Folder name where to store the images if they are going to be stored locally
	FString SaveLocallyFolderName;

	//
	int32 EntityIndex;

	//
	int32 SkelIndex;

	//
	FIntPoint Resolution;
};
