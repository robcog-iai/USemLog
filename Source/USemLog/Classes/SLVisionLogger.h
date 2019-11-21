// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

#if SL_WITH_LIBMONGO_C
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
struct FSLVisionEpisodeFrame
{
	// Frame timestamp
	float Timestamp;

	// Entity poses
	TMap<FString, FTransform> EntityPoses;

	// Skeletal poses
	TMap<FString, TMap<FString, FTransform>> SkeletalPoses;
};

/**
* The whole episode data
*/
struct FSLVisionEpisode
{
	// All the frames from the episode
	TArray<FSLVisionEpisodeFrame> Frames;

	// Id to static mesh actor
	TMap<FString, AStaticMeshActor*> IdToSMA;

	// Id to skeletal mesh actor
	TMap<FString, ASkeletalMeshActor*> IdToSkMA;

	
	//TMap<AActor*, FString> EntityIdMap;

	//TMap<
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
	
	// Connect to the database
	bool Connect(const FString& DBName, const FString& CollName, const FString& ServerIp, uint16 ServerPort);

	// Disconnect and clean db connection
	void Disconnect();

	// Get episode data from the database
	bool GetEpisodeData();

	// Disable physics and set to movable
	void DisablePhysics();

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
	int32 CurrVirtualCameraIdx;
	
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
