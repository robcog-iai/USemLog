// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLVizCineCamManager.generated.h"

// Forward declarations
class ACineCameraActor;
class ACameraActor;
class UCameraComponent;
class UCineCameraComponent;

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

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

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

	// Trigger effect on camera
	void ExecuteCameraEffect();

	// Change the lens focal point to create zoom
	void StartZoomEffect();

	// Stop the zoom effect animation
	void StopZoomEffect();

	// Zoom effect timer callback
	void ZoomTimerCallback();

private:
	// User input trigger action name (Shift+V)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	bool bIncludeBasicCameras = true;

	// User input trigger action name (Shift+V)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	FName UserInputActionName = "VizQCineCamTrigger";

	// Trigger the effect on the current camera
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	FName UserInputActionEffectName = "VizQCineCamEffectTrigger";

	// Re-load the cameras from the world
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ|Edit")
	bool bResetButton = true;

	// True if the active pawn is set
	bool bIsInit;

	// True if a camera effect is active
	bool bCameraEffectActive;

	// Flag for zooming in or out
	bool bZoomIn;

	// Timer handle for the camera effects
	FTimerHandle EffectsTimerHandle;

	// Active camera index
	int32 CurrCamIdx = INDEX_NONE;
	
	// Available cinematic cameras
	TArray<ACineCameraActor*> CineCameras;

	// Available cameras
	TArray<ACameraActor*> AllCameras;

	// Active camera component
	UCameraComponent* ActiveCameraComp = nullptr;

	// Active camera as cine camera component
	UCineCameraComponent* ActiveCineCameraComp = nullptr;

	// Cache the input binding to avoid setting it twice (due to reset capabilities)
	bool bActionInputBindingSet = false;
};
