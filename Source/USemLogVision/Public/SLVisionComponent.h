// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "SLVisionComponent.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics, Collision, LOD, AssetUserData))
class USEMLOGVISION_API USLVisionComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLVisionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Init component
	void Init();

	// Start capturing
	void Start();

	// Stop recording
	void Finish();

private:
#if WITH_EDITOR
	// Location and orientation visualization of the component
	class UArrowComponent* ArrowVis;
#endif // WITH_EDITOR

	// Set to true in order to edit the episode id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomCameraId;

	// Camera name
	UPROPERTY(EditAnywhere, Category = "Vision Settings", meta = (editcondition = "bUseCustomCameraId"))
	FString CameraId;

	// If false the viewport resolution will be used
	UPROPERTY(EditAnywhere, Category = "Vision Settings")
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


	// Camera capture component for color images (RGB)
	FViewport* ColorViewport;
	USceneCaptureComponent2D* ColorImgCaptureComp;

	// Camera capture component for depth images (Depth)
	USceneCaptureComponent2D* DepthImgCaptureComp;

	// Camera capture component for object mask
	USceneCaptureComponent2D* MaskImgCaptureComp;

	// Camera capture component for Normal images (Normal)
	USceneCaptureComponent2D* NormalImgCaptureComp;
};
