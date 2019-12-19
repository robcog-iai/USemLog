// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLMetaScannerStructs.h"
#include "SLMetaScannerHandler.h"
#include "SLMetaScanner.generated.h"

// Forward declarations
class USLMetadataLogger; // Parent
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UGameViewportClient;
class AStaticMeshActor;
class APlayerCameraManager;

/**
 * Scans handheld items by taking images from unidistributed points form a sphere as a camera location
 */
UCLASS()
class USLMetaScanner : public UObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLMetaScanner();

	// Dtor
	~USLMetaScanner();

	// Setup scanning room
	void Init(const FString& InTaskId, FSLItemScanParams ScanParams);

	// Start scanning, set camera into the first pose and trigger the screenshot
	void Start();

	// Finish scanning
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Load scan camera convenience actor
	bool LoadScanCameraPoseActor();

	// Load scanning points
	bool LoadScanPoses(int32 NumScanPose, float DistanceToCamera);

	// Load items to scan
	bool LoadScanItems(float MaxVolume = 0.f, float MaxBoundsLength = 0.f, bool bWithContactShape = false);

	// Load mask dynamic material
	bool LoadMaskMaterial();

	// Create clones of the items with mask material on top
	bool CreateMaskClones();

	// Init hi-res screenshot resolution
	void InitScreenshotResolution(FIntPoint Resolution);

	// Init render parameters (resolution, view mode)
	void InitRenderParameters();

	// Setup view mode
	bool SetupFirstViewMode();

	// Setup next view mode
	bool SetupNextViewMode();
	
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

	// Apply view mode
	void ApplyViewMode(ESLMetaScannerViewMode Mode);

	// Apply mask material to current item
	void ApplyMaskMaterial();

	// Apply original material to current item
	void ApplyOriginalMaterial();

	// Clean exit, all the Finish() methods will be triggered
	void QuitEditor();


	/* Helpers */
	// Get the bounding box and the number of pixels the item occupies in the image
	void GetItemPixelNumAndBB(const TArray<FColor>& InBitmap, int32 Width, int32 Height, int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax);

	// Get the bounding box and the number of pixels the color occupies in the image
	void GetColorPixelNumAndBB(const TArray<FColor>& InBitmap, const FColor& Color, int32 Width, int32 Height, int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax);

	// Get the number of pixels that the item occupies in the image
	int32 GetItemPixelNum(const TArray<FColor>& Bitmap);

	// Get the number of pixels of the given color in the image
	int32 GetColorPixelNum(const TArray<FColor>& Bitmap, const FColor& Color) const;
	
	// Count and check validity of the number of pixels the item represents in the image;
	void CountItemPixelNumWithCheck(const TArray<FColor>& Bitmap, int32 ResX, int32 ResY);
	
	// Get the number of pixels of the given two colors in the image
	void GetColorsPixelNum(const TArray<FColor>& Bitmap, const FColor& ColorA, int32& OutNumA, const FColor& ColorB, int32& OutNumB);

	// Get view mode name
	FString GetViewModeName(ESLMetaScannerViewMode Mode) const;
	
	// Get the camera distance for the selected item relative to its size
	float GetItemRelativeCameraDistance(UStaticMeshComponent* SMC) const;

	// Check against various properties if the item should be scanned
	bool HasScanningRequirements(UStaticMeshComponent* SMC, float MaxVolume, float MaxBoundsLength,
		bool bWithSemanticContactShape = false, bool bOnlyMovable = false) const;
	
	// Check if the item is wrapped in a semantic contact shape (has a SLContactShapeInterface sibling)
	bool HasSemanticContactShape(UStaticMeshComponent* SMC) const;

	// Generate sphere scan poses
	void GenerateSphereScanPoses(uint32 MaxNumOfPoints, float Radius, TArray<FTransform>& OutTransforms);

	// Print progress
	void PrintProgress() const;
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Scanner image handler
	FSLMetaScannerHandler ImgHandler;

	// Contains the data of the current scan in a given camera pose
	FSLScanPoseData ScanPoseData;

	// Flag to save the scanned images locally as well
	bool bIncludeLocally;

	// Pointer to the parent, used for updating the metadata mongo document;
	USLMetadataLogger* MetadataLoggerParent;
	
	// Location on where to save the data
	FString FolderName;

	// Current name of scan (for saving locally, and progress update purposes)
	FString CurrScanName;

	// Current scan view mode postfix
	FString ViewModePostfix;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY() // Avoid GC
	AStaticMeshActor* CameraPoseActor;

	// Dynamic mask material
	UPROPERTY() // Avoid GC
	UMaterialInstanceDynamic* DynamicMaskMaterial;

	// Original material of the item
	TArray<UMaterialInterface*> OriginalMaterials;
	
	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Scan camera poses
	TArray<FTransform> ScanPoses;

	// Scan item mesh with class name
	TArray<TPair<UStaticMeshComponent*, FString>> ScanItems;
	
	// View modes (lit/unlit/mask etc.)
	TArray<ESLMetaScannerViewMode> ViewModes;
	
	// Currently active view mode
	int32 CurrViewModeIdx;
	
	// Currently active camera pose scan index
	int32 CurrPoseIdx;

	// Currently scanned item index in map
	int32 CurrItemIdx;
	
	// Actor clones with mask material (avoids switching materials, which can lead to some texture artifacts)
	UPROPERTY() // Avoid GC
	TArray<AActor*> MaskClones;

	// True if the camera distance should be relative to the item size
	bool bWithItemRelativeCameraDistance;

	// Value of the camera distance for the current item
	float CurrItemCameraDistance;

	// Cache the previous view mode
	ESLMetaScannerViewMode PrevViewMode;

	// Currently counted number of pixels of the item
	int32 TempItemPixelNum;

	/* Constants */
	//// Volume limit in cubic centimeters (1000cm^3 = 1 Liter) of items to scan
	//constexpr static const float VolumeLimitConst = 14000.f;

	//// Length limit of its bounding box points (cm) 
	//constexpr static const float LengthLimitConst = 75.f;

	// Mask color
	constexpr static const FColor MaskColorConst = FColor(255,255,255);

	// Scan items only with SL Contact components
	constexpr static const bool bScanItemsOnlyWithSLContact = false;
	
	//// Distance to scan camera (cm)
	//constexpr static float DistanceToCameraConst = 25.f;
	
};
