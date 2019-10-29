// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "SLStructs.h" // FSLEntity
#include "SLItemScanner.generated.h"

// Forward declarations
class UGameViewportClient;
class AStaticMeshActor;
class APlayerCameraManager;

/**
* View modes
*/
UENUM()
enum class ESLItemScannerViewMode : uint8
{
	Lit						UMETA(DisplayName = "Lit"),
	Unlit					UMETA(DisplayName = "Unlit"),
	Mask					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
	Specular				UMETA(DisplayName = "Specular"),
};

/**
 * Scans handheld items by taking images from unidistributed points form a sphere as a camera location
 */
UCLASS()
class USLItemScanner : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLItemScanner();

	// Dtor
	~USLItemScanner();

	// Setup scanning room
	void Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort,
		 UWorld* InWorld, FIntPoint Resolution, bool bScanViewModeUnlit, bool bIncludeScansLocally, bool bOverwrite);

	// Start scanning
	void Start();

	// Finish scanning
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	/** Begin FTickableGameObject interface */
	// Called after ticking all actors, DeltaTime is the time passed since the last call.
	virtual void Tick(float DeltaTime) override;

	// Return if object is ready to be ticked
	virtual bool IsTickable() const override;

	// Return the stat id to use for this tickable
	virtual TStatId GetStatId() const override;
	/** End FTickableGameObject interface */

private:
	// Load scan box actor
	bool LoadScanBoxActor();

	// Load scan camera convenience actor
	bool LoadScanCameraPoseActor();

	// Load scanning points
	bool LoadScanPoints();

	// Load items to scan
	bool LoadItemsToScan();

	// Init render parameters (resolution, view mode)
	void InitRenderParameters(FIntPoint Resolution);

	// Setup view mode
	void SetupViewMode();
	
	// Move first item in the scan box
	bool SetupFirstScanItem();

	// Set next item in the scan box, return false if there are no more items
	bool SetupNextItem();

	// Move camera in position for the first scan
	bool SetupFirstScanPose();

	// Move camera in position for the next scan, return false if there are no more poses
	bool SetupNextScanPose();

	// Request a screenshot
	void RequestScreenshot();
	
	// Called when the screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);
	
	// Get the items to scan
	//void ScanItems(const float InVolumeLimit, const float InLengthLimit);

	// Check if the item should be scanned
	bool ShouldBeScanned(UStaticMeshComponent* SMC) const;

	// Clean exit, all the Finish() methods will be triggered
	void QuitEditor();
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// True if the object can be ticked (used by FTickableGameObject)
	bool bIsTickable;

	// View mode lit/unlit
	bool bUnlit;

	// Flag to save the scanned images locally as well
	bool bIncludeLocally;
	
	// Location on where to save the data
	FString Location;

	// Current name of scan (for saving locally, and progress update purposes)
	FString CurrScanName;

	// Scanner box actor to spawn
	UPROPERTY() // Avoid GC
	AStaticMeshActor* ScanBoxActor;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY() // Avoid GC
	AStaticMeshActor* CameraPoseActor;

	// Pointer to the world
	UWorld* World;
	
	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Scan camera poses
	TArray<FTransform> ScanPoses;

	// Scan items with semantic data
	TArray<TPair<AActor*, FSLEntity>> ScanItems;

	// Currently active camera pose scan index
	int32 CurrPoseIdx;

	// Currently scanned item index in map
	int32 CurrItemIdx;

	/* Constants */
	// Vertical offset to spawn the scanning room
	constexpr static const float ScanBoxOffsetZ = 1000.f;

	// Volume limit in cubic centimeters (1000cm^3 = 1 Liter) of items to scan
	constexpr static const float VolumeLimit = 40000.f;

	// Length limit of its bounding box points (cm) 
	constexpr static const float LengthLimit = 75.f;
};
