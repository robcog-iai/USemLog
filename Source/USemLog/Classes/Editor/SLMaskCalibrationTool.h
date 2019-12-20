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
	void Init(const FString& InFolderName = FString());

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

private:
	// Create mask clones of the available entities, hide everything else
	bool SetupWorld();

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

private:
	// Entity clones with mask material 
	UPROPERTY() // Avoid GC
	TMap<FColor, AStaticMeshActor*> EntityClones;

	// Skeletal clones with mask material 
	// TODO create a structure for the mapping
	//UPROPERTY() // Avoid GC
	//TMap<FColor, AStaticMeshActor*> SkelEntityClones;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY() // Avoid GC
	AStaticMeshActor* CameraPoseActor;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// Location on where to save the data locally (skip if empty)
	FString IncludeLocallyFolderName;

	// Current name of scan (for saving locally, and progress update purposes)
	FString CurrScanName;

};
