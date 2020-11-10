// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLImgScanner.generated.h"

// Forward declarations
class ASLIndividualManager;
class USLVisibleIndividual;
class AStaticMeshActor;
class UGameViewportClient;
class UMaterialInstanceDynamic;
class ADirectionalLight;

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Img Scanner")
class ASLImgScanner : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLImgScanner();

	// Dtor
	~ASLImgScanner();

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
	void AsyncScreenshotRequest();

	// Called when the screenshot is captured
	void ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap);

	// Hide all actors in the world
	void HideAllActors();

private:
	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Spawn a light actor which will also be used to move the camera around
	bool SetCameraPoseAndLightActor();

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
	float CameraLightIntensity = 1.6f;

	// Folder to store the images in
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString TaskId;

	// Mongo server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 MongoServerPort = 27017;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Convenience actor for setting the camera pose (SetViewTarget(InActor))
	UPROPERTY()
	ADirectionalLight* CameraPoseAndLightActor;

private:
	// Visual individuals to calibrate
	TArray<USLVisibleIndividual*> VisibleIndividuals;

	// Current individual index in the array
	int32 IndividualIdx = INDEX_NONE;

	// Used for triggering the screenshot request
	UGameViewportClient* ViewportClient;

	// The name of the current image
	FString CurrImageName;
};
