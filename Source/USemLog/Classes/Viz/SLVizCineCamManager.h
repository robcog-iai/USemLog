// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLVizCineCamManager.generated.h"

// Forward declarations
class ACineCameraActor;
class ACameraActor;

UCLASS(ClassGroup = (SL), DisplayName = "SL Viz CineCam Manager")
class ASLVizCineCamManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVizCineCamManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Init director references
	void Init();

	// Get init state
	bool IsInit() const { return bIsInit; };

protected:
	// Setup user input bindings
	void SetupInputBindings();

private:
	// Goto next camera
	void SwitchCamera();


private:
	// User input trigger action name (Shift+V)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	bool bIncludeBasicCameras = true;

	// User input trigger action name (Shift+V)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	FName UserInputActionName = "VizQCineCamTrigger";

	// True if the active pawn is set
	bool bIsInit;

	// Active camera index
	int32 CurrCamIdx = INDEX_NONE;
	
	// Available cinematic cameras
	TArray<ACineCameraActor*> CineCameras;

	// Available cameras
	TArray<ACameraActor*> AllCameras;
};
