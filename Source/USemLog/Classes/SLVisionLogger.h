// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

#include "Vision/SLVisionLoggerStructs.h"
#include "Vision/SLVisionPoseableMeshActor.h"

#if SL_WITH_LIBMONGO_C
class ASLVisionPoseableMeshActor;
THIRD_PARTY_INCLUDES_START
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <mongoc/mongoc.h>
#include "Windows/HideWindowsPlatformTypes.h"
#else
#include <mongoc/mongoc.h>
#endif // #if PLATFORM_WINDOWS
THIRD_PARTY_INCLUDES_END
#endif //SL_WITH_LIBMONGO_C
#include "SLVisionLogger.generated.h"

// Forward declarations
//class UMaterialInstanceDynamic;
class UGameViewportClient;
class ASLVisionCamera;

/**
 * Vision logger, can only be used with USemLogVision module
 * it saves the episode as a replay
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
		const FSLVisionLoggerParams& Params);

	// Start logger
	void Start(const FString& EpisodeId);

	// Finish logger
	void Finish(bool bForced = false);

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
	// Connect to the database
	bool Connect(const FString& DBName, const FString& CollName, const FString& ServerIp, uint16 ServerPort);

	// Disconnect and clean db connection
	void Disconnect();

	// Get episode data from the database (UpdateRate = 0 means all the data)
	bool LoadEpisode(float UpdateRate);

	// Prepare the world entities by disabling physics and setting them to movable
	void InitWorldEntities();
	
	// Create movable clones of the skeletal meshes, hide originals (call before loading the episode data)
	void CreatePoseableMeshes();

	// Load the pointers to the virtual cameras
	bool LoadVirtualCameras();

	// Create clones of the items with mask material on top, set them to hidden by default
	bool CreateMaskClones();

	// Init hi-res screenshot resolution
	void InitScreenshotResolution(FIntPoint Resolution);

	// Init render parameters (resolution, view mode)
	void InitRenderParameters();
	
	// Apply view mode
	void ApplyViewMode(ESLVisionLoggerViewMode Mode);

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

#if SL_WITH_LIBMONGO_C
	// Get the entities data out of the bson iterator, returns false if there are no entities
	bool GetEntityDataInFrame(bson_iter_t* doc, TMap<AActor*, FTransform>& OutEntityPoses) const;

	// Get the entities data out of the bson iterator, returns false if there are no entities
	bool GetSkeletalEntityDataInFrame(bson_iter_t* doc, TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>>& OutSkeletalPoses) const;
#endif //SL_WITH_LIBMONGO_C
	
protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// Current frame timestamp
	float CurrTimestamp;

	// Episode data to replay
	FSLVisionEpisode Episode;

	// Vision data in the current frame (timestamp)
	TArray<FSLVisionViewData> ViewsData;

	// Map from the skeletal entities to the poseable meshes
	UPROPERTY() // Avoid GC
	TMap<ASkeletalMeshActor*, ASLVisionPoseableMeshActor*> SkelToPoseableMap;

	// Copies of the static meshes with mask materials on top
	UPROPERTY() // Avoid GC
	TMap<AActor*, AStaticMeshActor*> OrigToMaskClones;

	// Copies of the (poseable) skeletal meshes with mask materials on top
	UPROPERTY() // Avoid GC
	TMap<ASLVisionPoseableMeshActor*, ASLVisionPoseableMeshActor*> SkelOrigToMaskClones;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Array of the virtual cameras
	TArray<ASLVisionCamera*> VirtualCameras;
	
	// View modes
	TArray<ESLVisionLoggerViewMode> ViewModes;
	
	// Currently active virtual camera view
	int32 CurrVirtualCameraIdx;
	
	// Currently active view mode
	int32 CurrViewModeIdx;

	// Previous view mode
	ESLVisionLoggerViewMode PrevViewMode;

	// Viewmode postfix (filename helpers)
	FString CurrViewModePostfix;

	// Image filename if it is going to be saved locally (plus a visual msg from editor when the image is saved)
	FString CurrImageFilename;

	// Folder name where to store the images if they are going to be stored locally
	FString TaskId;
	
#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;
#endif //SL_WITH_LIBMONGO_C	
};
