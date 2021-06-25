// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLCVMaskCalibrator.generated.h"

// Forward declarations
class ASLIndividualManager;
class USLVisibleIndividual;
class AStaticMeshActor;
class UStaticMesh;
class UGameViewportClient;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL CV Mask Calibrator")
class ASLCVMaskCalibrator : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLCVMaskCalibrator();

	// Dtor
	~ASLCVMaskCalibrator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Set up any required references and connect to server
	void Init();

	// Start processing any incomming messages
	void Start();

	// Stop processing the messages, and disconnect from server
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Request a high res screenshot
	void RequestScreenshotAsync();

	// Called when the screenshot is captured
	void ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap);

	// Set the next individual to calibrate
	bool SetNextView();

	// Get the calibrated color from the rendered screenshot image
	FString GetCalibratedMask(const TArray<FColor>& Bitmap);

	// Apply changes to the editor individual
	bool ApplyChangesToEditorIndividual(USLVisibleIndividual* VisibleIndividual);

	//  Quit the editor when finished
	void QuitEditor();

private:
	// Hide all actors in the world
	void HideAllActors();

	// Set screenshot image resolution
	void SetScreenshotResolution(FIntPoint Resolution);

	// Set the rendering parameters
	void SetRenderParams();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Spawn the canvas static mesh actor (mesh on which the mask colors will be applied)
	bool SetCanvasMeshActor();

	// Spawn a dummy actor to move the camera to
	bool SetCameraPoseActor();

protected:
	// Skip auto init and start
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// Save images to file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bSaveToFile : 1;

	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done 
	uint8 bIsFinished : 1;

	// Folder to store the images in
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString TaskId = "DefaultTaskId";

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Mesh used to load all the mask materials and rendered on screen
	UPROPERTY() 
	AStaticMeshActor* CanvasSMA;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY()
	AStaticMeshActor* CameraPoseActor;

	// Material used to render the masks
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaskMaterial;

	// Static mesh of the canvas
	UStaticMesh* CanvasSM;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

private:
	// Visual individuals to calibrate
	TArray<USLVisibleIndividual*> Individuals;

	// Current individual index in the array
	int32 ViewIdx = INDEX_NONE;

	// The name of the current image
	FString CurrImageName;

	/* Constants */
	static constexpr auto CanvasSMAssetPath = TEXT("/USemLog/CV/MaskRenderDummy/SM_MaskRenderDummy.SM_MaskRenderDummy");
	static constexpr auto CameraDummySMAssetPath = TEXT("/USemLog/CV/ScanCameraPoseDummy/SM_ScanCameraPoseDummy.SM_ScanCameraPoseDummy");
	static constexpr auto DynMaskMatAssetPath = TEXT("/USemLog/CV/M_SLDefaultMask.M_SLDefaultMask");
};
