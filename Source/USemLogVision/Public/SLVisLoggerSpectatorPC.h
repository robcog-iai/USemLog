// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
	// Called after a successful scrub
	void DemoGotoCB();

	// Called when screenshot is captured
	void ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

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

private:
	// Set when logger is initialized
	bool bIsInit;

	// Set when logger is started
	bool bIsStarted;

	// Set when logger is finished
	bool bIsFinished;

	// Pointer to the DemoNetDriver
	class UDemoNetDriver* NetDriver;

	// Pointer to the viewport
	class UGameViewportClient* ViewportClient;

	// Array of the camera actors
	TArray<class ASLVisCamera*> Cameras;

	// Path of the episode folder
	FString EpisodePath;

	// Index of the current view
	int32 ViewIndex;

	// Update step size
	float UpdateRate;
	
	// Current demo time
	float DemoTimeSeconds;

	// Number of saved images until now
	uint32 NumberOfSavedImages;

	// Total number of images to be saved
	uint32 NumberOfTotalImages;


	

private:
	//// User Input
	//// Go to next record step
	//void NextRecordStep();
	//// Go to prev record step
	//void PrevRecordStep();
	//// Go to next view
	//void NextCameraView();
	//// Go to previous camera view
	//void PrevCameraView();
	//// Take screenshot (curr view)
	//void RequestScreenshot();
	//void UpdateView();
//
//
//
//private:
//	//
//	float CurrDemoTime;
//	float TotalTime;
//	float UpdateRate;
//	int32 CurrViewIndex;
//	int32 TotalViews;





	//void GotoCbRecursive();

//void PauseToggle();

//void SetPause2(bool bPause);

//void Step();
//
//void NextView();

//


//bool TakeNextScreenshot();

//void StartCapture();

//void OnScrCb(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap);

};
