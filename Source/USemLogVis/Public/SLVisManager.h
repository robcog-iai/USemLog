// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLVisManager.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics, Collision, LOD, AssetUserData))
class USEMLOGVIS_API USLVisManager : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLVisManager();

	// Destructor
	~USLVisManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Init component
	void Init(const FString& InLogDir, const FString& InEpisodeId);

	// Start capturing
	void Start();

	// Stop recording
	void Finish();
	
private:
	// Called either from tick, or from the timer
	void Update();

private:
	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

#if WITH_EDITOR
	// Location and orientation visualization of the component
	class UArrowComponent* ArrowVis;
#endif // WITH_EDITOR

	// If false the viewport resolution will be used
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomResolution;

	// Camera Width
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (EditCondition = bUseCustomResolution))
	uint32 Width;

	// Camera Height
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (EditCondition = bUseCustomResolution))
	uint32 Height;

	// Camera field of view
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float FOV;

	// Camera update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate;

	// Capture Color image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Capture Mode")
	bool bCaptureColor;

	// Capture color images from the viewport
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Capture Mode|Color Mode", meta = (EditCondition = bCaptureColorImage))
	bool bCaptureColorFromViewport;

	// Capture Depth image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Capture Mode")
	bool bCaptureDepth;

	// Capture Mask image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Capture Mode")
	bool bCaptureMask;

	// Capture Normal image
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Capture Mode")
	bool bCaptureNormal;

	// Directory where to log
	FString LogDirectory;

	// Unique id of the episode
	FString EpisodeId;

	// Camera name
	FString CameraId;

	// Timer handle for custom update rate
	FTimerHandle TimerHandle;
};
