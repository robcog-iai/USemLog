// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

#include "Vision/SLVisionStructs.h"
#include "Vision/SLVisionPoseableMeshActor.h"
#include "Vision/SLVisionDBHandler.h"
#include "Vision/SLVisionImageHandler.h"
#include "Vision/SLVisionOverlapCalc.h"

#include "SLVisionLogger.generated.h"

// Forward declarations
class UGameViewportClient;
class ASLVisionCamera;

/**
 * Replays episodes from different perspectives and view modes,
 * while updating the data with vision related annotations
 */
UCLASS()
class USLVisionLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLVisionLogger();

	// Destructor
	~USLVisionLogger();

	// Init Logger
	void Init(const FString& InTaskId, const FString& InEpisodeId, const FString& InServerIp, uint16 InServerPort,
		bool bOverwriteVisionData, const FSLVisionLoggerParams& Params);

	// Start logger
	void Start(const FString& EpisodeId);

	// Finish logger
	void Finish(bool bForced = false);

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

	// Move actors to the poses from the first frame from the episode data
	bool SetupFirstEpisodeFrame();

	// Goto next episode frame, return false if there are no other left
	bool SetupNextEpisodeFrame();
	
	// Goto the first virtual camera view
	bool GotoFirstCameraView();

	// Goto next camera view, return false if there are no other left
	bool GotoNextCameraView();

	// Setup first view mode (render type)
	bool SetupFirstViewMode();

	// Setup next view mode (render type), return false if there are no other left
	bool SetupNextViewMode();

private:
	// Prepare the world entities by disabling physics and setting them to movable
	void InitWorldEntities();
	
	// Create movable clones of the skeletal meshes, hide originals (call before loading the episode data)
	void CreatePoseableMeshesClones();

	// Load the pointers to the virtual cameras
	bool LoadVirtualCameras();

	// Create clones of the items with mask material on top, set them to hidden by default
	bool CreateMaskClones();

	// Init hi-res screenshot resolution
	void InitScreenshotResolution(FIntPoint InResolution);

	// Init render parameters (resolution, view mode)
	void InitRenderParameters();
	
	// Apply view mode
	void ApplyViewMode(ESLVisionViewMode Mode);

	// Apply mask materials 
	void ApplyMaskMaterials();

	// Apply original material to current item
	void ApplyOriginalMaterials();

	// Clean exit, all the Finish() methods will be triggered
	void QuitEditor();
	
	// Save the compressed screenshot image locally
	void SaveImageLocally(const TArray<uint8>& CompressedBitmap);
	
	// Output progress to terminal
	void PrintProgress() const;

	// Get view mode as string
	FString GetViewModeName(ESLVisionViewMode Mode) const;

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// Writes and reads the data from the mongo database
	FSLVisionDBHandler DBHandler;

	// Gathers semantics from the images
	FSLVisionImageHandler ImgHandler;

	// Calculates entities overlap percentages in images
	UPROPERTY() // Avoid GC
	USLVisionOverlapCalc* OverlapCalc;

	// Current frame timestamp
	float CurrTimestamp;

	// Episode data to replay
	FSLVisionEpisode Episode;

	// Holds the vision data of all the views
	FSLVisionFrameData CurrFrameData;

	// Holds the current view vision data
	FSLVisionViewData CurrViewData;

	// Map from the skeletal entities to the poseable meshes
	UPROPERTY() // Avoid GC
	TMap<ASkeletalMeshActor*, ASLVisionPoseableMeshActor*> SkelToPoseableMap;

	// Copies of the static meshes with mask materials on top
	UPROPERTY() // Avoid GC
	TMap<AActor*, AStaticMeshActor*> OrigToMaskClones;

	// Copies of the (poseable) skeletal meshes with mask materials on top
	UPROPERTY() // Avoid GC
	TMap<ASLVisionPoseableMeshActor*, ASLVisionPoseableMeshActor*> SkelOrigToMaskClones;

	// Actors to hide when in mask mode
	TArray<AActor*> MaskViewModeBlacklistedActors;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Array of the virtual cameras
	TArray<ASLVisionCamera*> VirtualCameras;
	
	// View modes
	TArray<ESLVisionViewMode> ViewModes;
	
	// Currently active virtual camera view
	int32 CurrVirtualCameraIdx;
	
	// Currently active view mode
	int32 CurrViewModeIdx;

	// Previous view mode
	ESLVisionViewMode PrevViewMode;

	// Viewmode postfix (filename helpers)
	FString CurrViewModePostfix;

	// Image filename if it is going to be saved locally (plus a visual msg from editor when the image is saved)
	FString CurrImageFilename;

	// Folder name where to store the images if they are going to be stored locally
	FString SaveLocallyFolderName;

	// Image resolution 
	FIntPoint Resolution;
};
