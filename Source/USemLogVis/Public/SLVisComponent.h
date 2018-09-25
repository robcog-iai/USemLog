// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "RawDataAsyncWorker2.h"
#include "SLVisComponent.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent, DisplayName = "SL Vision Component"), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics, Collision, LOD, AssetUserData))
class USEMLOGVIS_API USLVisComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLVisComponent();

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

	// Change the framerate on the fly
	void SetFramerate(const float NewFramerate);

	// Initate AsyncTask
	void InitAsyncTask(FAsyncTask<RawDataAsyncWorker2>* AsyncWorker, TArray<FColor>& image, FDateTime Stamp, FString Folder, FString Name, int Width, int Height);

	// Start AsyncTask
	void CurrentAsyncTask(FAsyncTask<RawDataAsyncWorker2>* AsyncWorker);

	// Stop AsyncTask
	void StopAsyncTask(FAsyncTask<RawDataAsyncWorker2>* AsyncWorker);

private:
	// Timer callback (timer tick)
	void TimerTick();

	// Read Color raw data
	void ProcessColorImg();

	// Read Depth raw data
	void ProcessDepthImg();

	// Read Mask raw data
	void ProcessMaskImg();

	// Read Normal raw data
	void ProcessNormalImg();

	// ViewMode for ground truth types implemented with PostProcess material
	void PostProcess(FEngineShowFlags& ShowFlags);

	void BasicSetting(FEngineShowFlags& ShowFlags);

	// Use the source flags to set the object visibility of target */
	void SetVisibility(FEngineShowFlags& Target, FEngineShowFlags& Source);

	// ViewMode for object instance mask
	void VertexColor(FEngineShowFlags& ShowFlags);

	// Color All Actor in World
	bool ColorAllObjects();

	// Color each Actor with specific color
	bool ColorObject(AActor *Actor, const FString &name);

	// Generate Colors for each Actor
	void GenerateColors(const uint32_t NumberOfColors);

	// Read Raw data from FViewport
	void ReadPixels(FViewport *& viewport, TArray<FColor>& OutImageData, FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX), FIntRect InRect = FIntRect(0, 0, 0, 0));

	// Read Raw data from USceneCaptureComponent2D
	void ReadPixels(FTextureRenderTargetResource*& RenderResource, TArray< FColor >& OutImageData, FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX), FIntRect InRect = FIntRect(0, 0, 0, 0));

	void SetFileHandle(FString folder);

	void SetMaskColorFileHandle(FString folder);

#if WITH_EDITOR
	// Location and orientation visualization of the component
	class UArrowComponent* ArrowVis;
#endif // WITH_EDITOR

	// Set to true in order to edit the episode id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomCameraId;

	// Camera name
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseCustomCameraId"))
	FString CameraId;

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

	// Image Wrapper for saving image
	TSharedPtr<IImageWrapper> ImageWrapper;

	/** A fence which is used to keep track of the rendering thread releasing the geometry cache resources. */
	/** Fence used to track progress of render resource destruction. */
	/** Fence used to track progress of releasing resources on the rendering thread. */
	// See https://docs.unrealengine.com/en-US/Programming/Rendering/ThreadedRendering
	FRenderCommandFence PixelFence;


	// TODO big time rm
	// Array of objects' colors
	TArray<FColor> ObjectColors;

	// Array of Actors in world
	TArray<FString> ObjectCategory;

	// Map a specfic color for each paintable object
	TMap<FString, uint32> ObjectToColor;

	// number of Used Colors
	uint32 ColorsUsed;


	// helper bool for Reading and Saving 
	bool bFirsttick;
	bool bColorSave;
	bool bDepthSave;
	bool bMaskSave;
	bool bNormalSave;
	bool bConnectionMongo;


	//Async worker to save the color raw data
	FAsyncTask<RawDataAsyncWorker2>* ColorAsyncWorker;

	//Async worker to save the Depth raw data
	FAsyncTask<RawDataAsyncWorker2>* DepthAsyncWorker;

	//Async worker to save the Depth raw data
	FAsyncTask<RawDataAsyncWorker2>* MaskAsyncWorker;

	//Async worker to save the Normal raw data
	FAsyncTask<RawDataAsyncWorker2>* NormalAsyncWorker;

	// File handle to write the raw data to file
	IFileHandle* FileHandle;

	// File handle to write the raw data to file
	IFileHandle* MaskColorFileHandle;

	// Intial Asynctask
	bool bInitialAsyncTask;

};
