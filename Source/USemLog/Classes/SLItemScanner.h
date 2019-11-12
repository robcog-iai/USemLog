// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "CoreMinimal.h"
#include "SLStructs.h" // FSLEntity
#include "SLItemScanner.generated.h"

// Forward declarations
class USLMetadataLogger; // Parent
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UGameViewportClient;
class AStaticMeshActor;
class APlayerCameraManager;

/**
* View modes
*/
UENUM()
enum class ESLItemScannerViewMode : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Lit						UMETA(DisplayName = "Color (lit)"),
	Unlit					UMETA(DisplayName = "Color (unlit)"),
	Mask					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
};

/**
 * Scans handheld items by taking images from unidistributed points form a sphere as a camera location
 */
UCLASS()
class USLItemScanner : public UObject
{
	GENERATED_BODY()

public:
	// Ctor
	USLItemScanner();

	// Dtor
	~USLItemScanner();

	// Setup scanning room
	void Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort,
		FIntPoint InResolution, const TSet<ESLItemScannerViewMode>& InViewModes, bool bIncludeScansLocally);

	// Start scanning
	void Start(USLMetadataLogger* InParent);

	// Finish scanning
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	//// Load scan box actor
	//bool LoadScanBoxActor();

	// Load scan camera convenience actor
	bool LoadScanCameraPoseActor();

	// Load scanning points
	bool LoadScanPoints();

	// Load items to scan
	bool LoadScanItems(bool bWithContactShape = false);

	// Load mask dynamic material
	bool LoadMaskMaterial();

	// Init render parameters (resolution, view mode)
	void InitRenderParameters();

	// Setup view mode
	bool SetupFirstViewMode();

	// Setup next view mode (bCircular, if the last value is reached, jump to the first one)
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

	// Count the number of pixels the item represents in the image;
	void CountItemPixelNum(const TArray<FColor>& Bitmap);
	
	// Get the number of pixels of the given color in the image
	int32 GetColorPixelNum(const TArray<FColor>& Bitmap, const FColor& Color) const;
	
	// Get the number of pixels of the given two colors in the image
	void GetColorsPixelNum(const TArray<FColor>& Bitmap, const FColor& ColorA, int32& OutNumA, const FColor& ColorB, int32& OutNumB);
	
	// Apply view mode
	void ApplyViewMode(ESLItemScannerViewMode Mode);

	// Get view mode name
	FString GetViewModeName(ESLItemScannerViewMode Mode) const;

	// Apply mask material to current item
	void ApplyMaskMaterial();

	// Apply original material to current item
	void ApplyOriginalMaterial();
	
	// Check if the item should be scanned
	bool IsHandheldItem(UStaticMeshComponent* SMC) const;

	// Check if the item is wrapped in a semantic contact shape (has a SLContactShapeInterface sibling)
	bool HasSemanticContactShape(UStaticMeshComponent* SMC) const;
	
	// Clean exit, all the Finish() methods will be triggered
	void QuitEditor();

	// Generate sphere scan poses
	void GenerateSphereScanPoses(uint32 MaxNumOfPoints, float Radius, TArray<FTransform>& OutTransforms);
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Flag to save the scanned images locally as well
	bool bIncludeLocally;

	// Pointer to the parent, used for updating the metadata mongo document;
	USLMetadataLogger* Parent;
	
	// Location on where to save the data
	FString Location;

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
	TArray<ESLItemScannerViewMode> ViewModes;
	
	// Currently active view mode
	int32 CurrViewModeIdx;
	
	// Currently active camera pose scan index
	int32 CurrPoseIdx;

	// Currently scanned item index in map
	int32 CurrItemIdx;
	
	// TODO
	//// Actor duplicates with mask material (avoids switching materials, which can lead to some texture artifacts)
	//UPROPERTY() // Avoid GC
	//TArray<AActor*> MaskDuplicates;

	// Cache the previous view mode
	ESLItemScannerViewMode PrevViewMode;

	// Scan image resolution
	FIntPoint Resolution;

	// Currently counted number of pixels of the item
	int32 ItemPixelNum;

	/* Constants */
	// Volume limit in cubic centimeters (1000cm^3 = 1 Liter) of items to scan
	constexpr static const float VolumeLimit = 14000.f;

	// Length limit of its bounding box points (cm) 
	constexpr static const float LengthLimit = 75.f;

	// Number of scans
	constexpr static const int32 NumScanPoints = 2;
};
