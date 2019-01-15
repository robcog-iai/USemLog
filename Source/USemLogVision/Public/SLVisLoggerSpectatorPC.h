// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
	bool SetupViewType(FName ViewType);

	// Set the suffix of the file depending on the view type
	bool SetupFilenameSuffix(FName ViewType);

	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& IP, uint16 Port);

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

	// Buffer types to render
	TArray<FName> ViewTypes;

	// Saves the image data to file/database etc.
	UPROPERTY()
	TScriptInterface<ISLVisImageWriterInterface> Writer;	
	ISLVisImageWriterInterface* Writer2;

	// Path of the episode folder
	FString EpisodePath;

	// Suffix of the filename
	FString FilenameSuffix;

	// Index of the current view
	int32 ViewTargetIndex;

	// Index of the current view
	int32 ViewTypeIndex;

	// Update step size
	float UpdateRate;
	
	// Current demo time
	float DemoTimeSeconds;

	// Number of saved images until now
	uint32 NumberOfSavedImages;

	// Total number of images to be saved
	uint32 NumberOfTotalImages;
//
//#if WITH_LIBMONGO
//	// Must be created before using the driver and must remain alive for as long as the driver is in use
//	//mongocxx::instance mongo_inst;
//
//	// Mongo connection client
//	mongocxx::client mongo_conn;
//
//	// Database to access
//	mongocxx::database mongo_db;
//
//	// Database collection
//	mongocxx::collection mongo_coll;
//#endif //WITH_LIBMONGO
};
