// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "IImageWrapper.h"
#include "SLVisLegacyManager.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics, Collision, LOD, AssetUserData))
class USEMLOGVISION_API USLVisLegacyManager : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLVisLegacyManager();

	// Destructor
	~USLVisLegacyManager();

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

	// Read the image data
	void ReadData();

	// Read from viewport
	void ReadPixelsFromViewport(TArray<FColor>& OutImageData,
		FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
	
	// Read from scene capture component
	void ReadPixels(FTextureRenderTargetResource*& RenderResource, TArray<FColor>& OutImageData,
		FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));

	// Save the image data
	void SaveData();

	// Create capture components
	void CreateCaptureComponents();

	// Create color capture component
	void CreateColorCaptureComponent();

	// Create depth capture component
	void CreateDepthCaptureComponent();

	// Create mask capture component
	void CreateMaskCaptureComponent();

	// Create normal capture component
	void CreateNormalCaptureComponent();

	// Init capture components
	void InitCaptureComponents();

	// Init color capture component
	void InitColorCaptureComponent();

	// Init depth capture component
	void InitDepthCaptureComponent();

	// Init mask capture component
	void InitMaskCaptureComponent();

	// Init normal capture component
	void InitNormalCaptureComponent();

	// Add a mask color for each object
	void InitMaskColors();

	// Create an array of different colors
	void SetUniqueMaskColors(TMap<AStaticMeshActor*, FColor>& OutActorColorMap);

private:
	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

#if WITH_EDITORONLY_DATA
	// Location and orientation visualization of the component
	UPROPERTY()
	class UArrowComponent* ArrowVis;
#endif // WITH_EDITORONLY_DATA

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

	// Capture components
	// Camera capture component for color images (RGB)
	FViewport* ColorViewport;
	USceneCaptureComponent2D* ColorSceneCaptureComp;

	// Camera capture component for depth images (Depth)
	USceneCaptureComponent2D* DepthSceneCaptureComp;

	// Camera capture component for object mask
	USceneCaptureComponent2D* MaskSceneCaptureComp;

	// Camera capture component for Normal images (Normal)
	USceneCaptureComponent2D* NormalSceneCaptureComp;

	// Material instance to get the depth data
	UMaterial* MaterialDepthInstance;

	// Material instance to get the Normal data
	UMaterial* MaterialNormalInstance;

	// Image buffers
	// Color image buffer
	TArray<FColor> ColorImage;
	//TArray<uint8> ColorImageUint;

	// Depth image buffer
	TArray<FColor> DepthImage;
	//TArray<uint8> DepthImageUint;

	// Mask image buffer
	TArray<FColor> MaskImage;
	//TArray<uint8> MaskImageUint;

	// Normal image buffer
	TArray<FColor> NormalImage;
	//TArray<uint8> NormalImageUint;

	// Image Wrapper for saving the images
	TSharedPtr<IImageWrapper> ImageWrapper;

	// Used to track pending rendering commands from the game thread.
	FRenderCommandFence PixelFence;

	// Count for saved images
	uint32 ImgTickCount;
};
