// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLogVision.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopedSlowTask.h"
#include "SLVisImageWriterInterface.h"
#include "SLVisHelpers.h"
#include "SLVisLoggerSpectatorPC.generated.h"

/**
 * Player controller stepping through the saved demo
 */
UCLASS()
class USEMLOGVISION_API ASLVisLoggerSpectatorPC : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Ctor
	ASLVisLoggerSpectatorPC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Initialize logger
	void Init();

	// Start logger
	void Start();

	// Finish logger
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };
	
private:
	// Create data writer
	void CreateWriter();

	// Cache existing camera views
	void SetCameraViews();

	// Set rendered image quality
	void SetRenderingParameters();

	// Request a screenshot
	void RequestScreenshot();

	// Called after a successful scrub
	void ScrubCB();

	// Called when screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

	// Called when the replay is finished
	void DemoFinishedCB();

	// Pause demo
	void DemoPause();

	// Un-pause demo
	void DemoUnPause();

	// Sets the first view, returns false if there are no views at all
	bool GotoInitialViewTarget();

	// Sets the next view, returns false there are no more views
	bool GotoNextViewTarget();

	// Sets the first visualization buffer type, false if none
	bool GotoInitialRenderType();

	// Sets the next visualization buffer type, returns false there are no more views
	bool GotoNextRenderType();

	// Setup the given view type
	bool ApplyRenderType(ESLVisRenderType ViewType);

	// Called when the demo reaches the last frame
	void QuitEditor();

	// Skip the current timestamp (return false if not a mongo writer)
	bool ShouldSkipThisFrame(float Timestamp);

	// Check if entities are in the frustum using bounding box checks
	void ShallowFrustumCheck();

	// Check if entities are rendered using the image masks
	void MaskFrustumCheck();

private:
	// Set when logger is initialized
	bool bIsInit;

	// Set when logger is started
	bool bIsStarted;

	// Set when logger is finished
	bool bIsFinished;

	// Saves the image data to file/database etc.
	UPROPERTY() // TScriptInterface + UPROPERTY avoids GC on interfaces
	TScriptInterface<ISLVisImageWriterInterface> Writer;

	// Mask image visualizer helper
	UPROPERTY() // Avoid GC
	class USLVisMaskHandler* MaskHandler;

	// Pointer to the DemoNetDriver
	class UDemoNetDriver* NetDriver;

	// Pointer to the viewport
	class UGameViewportClient* ViewportClient;

	// Cache data of the current view
	FSLVisViewData CurrentViewData;

	// Cache data of the current timestamp
	FSLVisStampedData CurrentTsData;

	// Array of the views
	TArray<class ASLVisViewActor*> CameraViews;

	// Rendering buffer types
	TArray<ESLVisRenderType> RenderTypes;

	// Index of the current view
	int32 CurrentViewIndex;

	// Index of the current render type
	int32 CurrRenderIndex;

	// Screenshot resolution
	FIntPoint Resolution;

	// Update rate of the replay in seconds
	float ScrubRate;
	
	// Avoid creating a new db entry if there is a world state in the given range [--- Ts ---]
	float SkipNewEntryTolerance;

	// Current demo time
	float DemoTimestamp;

	// Progress logger helper class
	FSLVisProgressLogger ProgressLogger;

	// Process duration logger helper class
	FSLVisDurationsLogger DurationsLogger;

#if WITH_EDITOR
	// Progress bar
	//TUniquePtr<FScopedSlowTask> ProgressBar;
#endif //WITH_EDITOR

	/* Constants */
	// Database name
	constexpr static const char* DBName = "SemLogVis421";
};
