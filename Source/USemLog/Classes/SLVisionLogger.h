// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

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
class UMaterialInstanceDynamic;
class UGameViewportClient;
class ASLVisionCamera;

/**
* View modes
*/
UENUM()
enum class ESLVisionLoggerViewMode : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Color					UMETA(DisplayName = "Color"),
	Unlit					UMETA(DisplayName = "Unlit"),
	Mask					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
};

/**
* Vision logger parameters
*/
struct FSLVisionLoggerParams
{
	// Update rate
	float UpdateRate;

	// Resolution
	FIntPoint Resolution;

	// Default ctor
	FSLVisionLoggerParams();

	// Init ctor
	FSLVisionLoggerParams(
		float InUpdateRate,
		FIntPoint InResolution) :
		UpdateRate(InUpdateRate),
		Resolution(InResolution)
	{};
};

/**
* Episode frame data
*/
struct FSLVisionFrame
{
	// Frame timestamp
	float Timestamp;

	// Entity poses
	TMap<AActor*, FTransform> ActorPoses;

	// Skeletal (poseable) meshes bone transformation
	TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>> PMActorPoses;

	// Apply transformations, return the timestamp
	float ApplyTransformations()
	{
		// Move the static meshes
		for(const auto& Pair : ActorPoses)
		{
			Pair.Key->SetActorTransform(Pair.Value);
		}

		// Move the skeletal(poseable) meshes
		for(const auto& Pair : PMActorPoses)
		{
			Pair.Key->SetBoneTransforms(Pair.Value);
		}
		return Timestamp;
	}
};

/**
* The whole episode data
*/
struct FSLVisionEpisode
{
	// All the frames from the episode
	TArray<FSLVisionFrame> Frames;

	// Default ctor
	FSLVisionEpisode() : FrameIdx(INDEX_NONE) {};

	// Move actors to the first frame
	bool GotoFirstFrame(float& OutTimestamp)
	{
		FrameIdx = 0;
		if(Frames.IsValidIndex(FrameIdx))
		{
			OutTimestamp = Frames[FrameIdx].ApplyTransformations();
			return true;
		}
		return false;
	}

	// Move actors to the next frame transformations, return false if no more frames are available
	bool GotoNextFrame(float& OutTimestamp)
	{
		FrameIdx++;
		if(Frames.IsValidIndex(FrameIdx))
		{
			OutTimestamp = Frames[FrameIdx].ApplyTransformations();
		}
		FrameIdx = INDEX_NONE;
		return false;
	}
	
private:
	// Current frame index
	int32 FrameIdx;
};


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

private:
	// Called when the screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

	// Goto the first episode frame
	bool GotoFirstEpisodeFrame();

	// Goto next episode frame, return false if there are no other left
	bool GotoNextEpisodeFrame();
	
	// Goto the first virtual camera view
	bool GotoFirstCameraView();

	// Goto next camera view, return false if there are no other left
	bool GotoNextCameraView();

	// Goto first view mode (render type)
	bool GotoFirstViewMode();

	// Goto next view mode (render type), return false if there are no other left
	bool GotoNextViewMode();
	
	// Connect to the database
	bool Connect(const FString& DBName, const FString& CollName, const FString& ServerIp, uint16 ServerPort);

	// Disconnect and clean db connection
	void Disconnect();

	// Get episode data from the database
	bool GetEpisodeData();

	// Prepare th world entities by disabling physics and setting them to movable
	void SetupWorldMobilty();
	
	// Setup the poseable mesh clones for the skeletal ones
	void SetupPoseableMeshes();

	// Load the pointers to the virtual cameras
	bool LoadVirtualCameras();

	// Load mask dynamic material
	bool LoadMaskMaterial();

	// Create clones of the items with mask material on top
	bool SetupMaskClones();

	// Init hi-res screenshot resolution
	void InitScreenshotResolution(FIntPoint Resolution);

	// Init render parameters (resolution, view mode)
	void InitRenderParameters();

	// Request a screenshot
	void RequestScreenshot();
	
	// Apply view mode
	void ApplyViewMode(ESLVisionLoggerViewMode Mode);

	// Apply mask materials 
	void ApplyMaskMaterials();

	// Apply original material to current item
	void ApplyOriginalMaterials();
	
	// Clean exit, all the Finish() methods will be triggered
	void QuitEditor();
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Episode data to replay
	FSLVisionEpisode EpisodeData;

	// Map from the skeletal entities to the poseable meshes
	UPROPERTY() // Avoid GC
	TMap<ASkeletalMeshActor*, ASLVisionPoseableMeshActor*> SkMAToPMA;
	
	// Dynamic mask material
	UPROPERTY() // Avoid GC
	UMaterialInstanceDynamic* DynamicMaskMaterial;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Array of the virtual cameras
	TArray<ASLVisionCamera*> VirtualCameras;
	
	// View modes
	TArray<ESLVisionLoggerViewMode> ViewModes;

	// Currently active virtual camera view
	int32 CurrCameraIdx;
	
	// Currently active view mode
	int32 CurrViewModeIdx;
	
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
