// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Vision/SLVisionStructs.h"
#include "SLVisionOverlapCalc.generated.h"

// Forward declaration
class UGameViewportClient;
class USLVisionLogger;
class AStaticMeshActor;
class UMaterialInterface;
class UMaterial;
//class USLSkeletalDataComponent;

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
	void Init(USLVisionLogger* InParent, FIntPoint InResolution, const FString& InSaveLocallyPath = FString());

	// Calculate overlaps for the given scene
	void Start(struct FSLVisionViewData* CurrViewData, float Timestamp, int32 FrameIdx);

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

	// Select the first skel bone in the array
	bool SelectFirstSkelBone();

	// Select the next skeletal bone in the array (if available)
	bool SelectNextSkelBone();

	// Apply the non occluding material to the currently selected item
	void ApplyNonOccludingMaterial();

	// Re-apply the original material to the currently selected item
	void ReApplyOriginalMaterial();

	// Calculate overlap
	void CalculateOverlap(const TArray<FColor>& NonOccludedImage, int32 ImgWidth, int32 ImgHeight);

	// Print out the progress in the terminal
	void PrintProgress() const;

	/* Helper */
	// Return INDEX_NONE if not possible
	int32 GetMaterialIndexOfCurrentlySelectedBone();

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

	// True if the active item is a bone
	bool bSkelBoneActive;

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

	// Subfolder name for saving locally
	FString SubFolderName;

	// Index of the entity in the array (INDEX_NONE if not active/set)
	int32 EntityIndex;

	// Index of the skel entity in the array (INDEX_NONE if not active/set)
	int32 SkelIndex;

	// Index of the skel entity in the array (INDEX_NONE if not active/set)
	int32 BoneIndex;

	// Pointer to the current mask clone
	AStaticMeshActor* CurrSMAClone;

	// Pointer to the current skel mask clone
	ASLVisionPoseableMeshActor* CurrPMAClone;

	//// Currently active skeletal data component
	//USLSkeletalDataComponent* CurrSkelDataComp;

	// Currently selected bone material index
	int32 CurrBoneMaterialIndex;

	// Screenshor resolution for the overlap calculation images (usually lower than the visual logger)
	FIntPoint Resolution;

	// Current overlap frame, every screenshot counts as a frame
	int32 CurrOverlapCalcIdx;

	// Total number of overlap frames
	int32 TotalOverlapCalcNum;

	// Current timestamp from the vision logger
	float CurrTs;

	// Current frame index from the vision logger
	int32 CurrFrameIdx;
};
