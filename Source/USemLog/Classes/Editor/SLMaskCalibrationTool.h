// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLMaskCalibrationTool.generated.h"

// Forward declarations
class ASkeletalMeshActor;
class AStaticMeshActor;
class UGameViewportClient;
class UMaterialInstanceDynamic;

/**
 * Renders the masked materials on the items and 
 * compares the rendered pixel values with the original ones
 */
UCLASS()
class USLMaskCalibrationTool : public UObject
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLMaskCalibrationTool();

	// Dtor
	~USLMaskCalibrationTool();

	// Setup scanning room
	void Init(bool bMaskColorsOnlyDemo = false, const FString& InFolderName = FString());

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

protected:
	// Request a screenshot
	void RequestScreenshot();

	// Called when the screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

	// Render the color of te first mask
	bool SetupFirstMaskColor();

	// Render the color of te next mask
	bool SetupNextMaskColor();

	// Return the actual mask rendered color
	FColor GetRenderedColor(const TArray<FColor>& Bitmap);

	// Store the rendered color
	bool StoreRenderedColor(const FColor& InRenderedColor);

private:
	// Init hi-res screenshot resolution
	void InitScreenshotResolution(FIntPoint InResolution);

	// Init render parameters (resolution, view mode)
	void InitRenderParameters();

	// Load mesh that will be used to render all the mask colors on screen
	bool CreateMaskRenderMesh();

	// Load camera convenience actor
	bool CreateTargetCameraPoseActor();

	// Load the mask colors to their entities mapping (while hiding all the actors in the world)
	bool LoadMaskMappings();

	// Render the color of te first entity mask
	bool SetupFirstEntityMaskColor();

	// Render the color of te next entity mask
	bool SetupNextEntityMaskColor();

	// Render the color of te firt skel mask
	bool SetupFirstSkelMaskColor();

	// Render the color of te next skel mask
	bool SetupNextSkelMaskColor();

	// Output progress to terminal
	void PrintProgress(AActor* Parent, FColor OrigColor, FColor RenderedColor, FString = "") const;

	// Quit the editor once the scanning is finished
	void QuitEditor();

	/* Legacy */
	// Create mask clones of the available entities, hide everything else
	bool SetupMaskColorsWorld(bool bOnlyDemo = false);

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// True if the skel entities are started
	bool bSkelMasksActive;

	// Mesh used to load all the mask materials and rendered on screen
	UPROPERTY() // Avoid GC
	AStaticMeshActor* MaskRenderActor;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY() // Avoid GC
	AStaticMeshActor* CameraPoseActor;

	// Material to apply to the render mesh
	UPROPERTY() // Avoid GC
	UMaterialInstanceDynamic* DynamicMaskMaterial;

	// Pointer to the render SMA mesh
	class UStaticMesh* MaskRenderMesh;

	// Mask color to actor mapping
	TArray<TPair<FColor, AStaticMeshActor*>> MaskToEntity;

	// Mask color to skeletal actor bone mapping
	TArray<TPair<FColor, TPair<ASkeletalMeshActor*, FName>>> MaskToSkelAndBone;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Location on where to save the data locally (skip if empty)
	FString IncludeLocallyFolderName;

	// Current name of scan (for saving locally, and progress update logging)
	FString CurrImgName;

	// Currently active item index in map
	int32 CurrEntityIdx;

	// Currently active skeletal item index in map
	int32 CurrSkelIdx;

	/* Legacy */
	// Map from the cloned actors to the real ones
	TArray<TPair<AStaticMeshActor*, AStaticMeshActor*>> CloneToRealArray;
};
