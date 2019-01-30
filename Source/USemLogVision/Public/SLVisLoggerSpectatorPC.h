// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopedSlowTask.h"
#include "SLVisImageWriterInterface.h"
#include "SLVisLoggerSpectatorPC.generated.h"


/**
 * 
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

	// Setup user input bindings
	virtual void SetupInputComponent() override;

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
	// Set rendered image quality
	void SetupRenderingProperties();

	// Called after a successful scrub
	void DemoGotoCB();

	// Called when screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

	// Called when the demo reaches the last frame
	void QuitEditor();

	// Pause demo
	void DemoPause();

	// Un-pause demo
	void DemoUnPause();

	// Check if demo is paused
	bool IsDemoPaused();

	// Sets the first view, returns false if there are no views at all
	bool SetFirstViewTarget();

	// Sets the next view, returns false there are no more views
	bool SetNextViewTarget();

	// Sets the first visualization buffer type, false if none
	bool SetFirstViewType();

	// Sets the next visualization buffer type, returns false there are no more views
	bool SetNextViewType();

	// Setup the given view type
	bool ChangeViewType(const FName& ViewType);

	// Skip the current timestamp (return false if not a mongo writer)
	bool ShouldSkipThisTimestamp(float Timestamp);

private:
	// Set when logger is initialized
	bool bIsInit;

	// Set when logger is started
	bool bIsStarted;

	// Set when logger is finished
	bool bIsFinished;

	// Saves the image data to file/database etc.
	UPROPERTY() // TScriptInterface can be used with UPROPERTY to avoid GC
	TScriptInterface<ISLVisImageWriterInterface> Writer;

	// Images at a given timeslice
	TArray<FSLVisImageData> ImagesAtTimestamp;

	// Pointer to the DemoNetDriver
	class UDemoNetDriver* NetDriver;

	// Pointer to the viewport
	class UGameViewportClient* ViewportClient;

	// Array of the camera actors
	TArray<class ASLVisCameraView*> CameraViews;

	// Rendering buffer types
	TArray<FName> ViewTypes;

	// Index of the current view
	int32 ActiveCameraViewIndex;

	// Index of the current view type
	int32 ActiveViewTypeIndex;

	// Image size X
	int32 ResX;

	// Image size Y
	int32 ResY;

	// Update rate in seconds
	float DemoUpdateRate;
	
	// Current demo time
	float DemoTimestamp;

	// Number of saved images until now
	uint32 NumImagesSaved;

	// Total number of images to be saved
	uint32 NumImagesToSave;

#if WITH_EDITOR
	// Progress bar
	//TUniquePtr<FScopedSlowTask> ProgressBar;
#endif //WITH_EDITOR
};
