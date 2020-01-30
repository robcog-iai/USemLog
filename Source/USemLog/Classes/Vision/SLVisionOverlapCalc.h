// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisionOverlapCalc.generated.h"

// Forward declaration
class UGameViewportClient;
class USLVisionLogger;
class AStaticMeshActor;
class UMaterialInterface;

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

	// Calculate overlaps for the given scene
	void Start(struct FSLVisionViewData* CurrViewData);

	// Reset all flags and temporaries, called when the scene overlaps are calculated, this un-pauses the parent as well
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

	// Select the first item (static or skeletal)
	bool SelectFirstItem();

	// Select the next item (static or skeletal), return false when no more items are available
	bool SelectNextItem();

private:
	// Select the first entity in the array (if not empty)
	bool SelectFirstEntity();

	// Select the next entity in the array (if available)
	bool SelectNextEntity();

	// Select the first skel entity in the array (if not empty)
	bool SelectFirstSkel();

	// Select the next skeletal entity in the array (if available)
	bool SelectNextSkel();

	// Apply the non occluding material to the currently selected item
	void ApplyNonOccludingMaterial();

	// Re-apply the original material to the currently selected item
	void ReApplyOriginalMaterial();

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

	UPROPERTY() // Avoid GC
	UMaterial* DefaultNonOccludingMaterial;

	// Chached mask materials of the active instance
	UPROPERTY() // Avoid GC
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

	// Pointer to the current mask clone
	AStaticMeshActor* CurrSMAClone;

	// Pointer to the current skel mask clone
	ASLVisionPoseableMeshActor* CurrPMAClone;

	//
	FIntPoint Resolution;
};
