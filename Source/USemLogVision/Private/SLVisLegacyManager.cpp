// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLegacyManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Misc/FileHelper.h"
#include "IImageWrapperModule.h"
#if WITH_EDITOR
#include "Components/ArrowComponent.h"
#endif // WITH_EDITOR

// Sets default values for this component's properties
USLVisLegacyManager::USLVisLegacyManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// Disable tick, if needed, it will be enabled when the framerate is set
	PrimaryComponentTick.bStartWithTickEnabled = false;

#if WITH_EDITORONLY_DATA
	ArrowVis = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("SLVisArrowComponent"));
	if (ArrowVis)
	{
		ArrowVis->SetupAttachment(this);
		ArrowVis->ArrowColor = FColor::Red;
		ArrowVis->bTreatAsASprite = true;
		ArrowVis->bLightAttachment = true;
		ArrowVis->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA

	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Default parameters
	CameraId = TEXT("autogen"); // TODO use tags
	bUseCustomResolution = true;
	Width = 480;
	Height = 320;
	FOV = 90.0;
	UpdateRate = 0.f; // 0.f = update as often as possible (boils down to every tick)
	bCaptureColor = true;
	bCaptureColorFromViewport = false;
	bCaptureDepth = true;
	bCaptureMask = true;
	bCaptureNormal = true;

	ImgTickCount = 0;

	// Setup capture components
	USLVisLegacyManager::CreateCaptureComponents();
}

// Destructor
USLVisLegacyManager::~USLVisLegacyManager()
{
	if (!bIsFinished)
	{
		USLVisLegacyManager::Finish();
	}
}

// Called when the game starts
void USLVisLegacyManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void USLVisLegacyManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Call update on tick
	USLVisLegacyManager::Update();
}

// Init component
void USLVisLegacyManager::Init(const FString& InLogDir, const FString& InEpisodeId)
{
	if (!bIsInit)
	{
		LogDirectory = InLogDir;
		EpisodeId = InEpisodeId;
	
		// Init capture components
		USLVisLegacyManager::InitCaptureComponents();

		// Mark manager as initialized
		bIsInit = true;
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	}
}

// Start capturing
void USLVisLegacyManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (UpdateRate > 0.0f)
		{
			// Update logger on custom timer tick (does not guarantees the UpdateRate value,
			// since it will be eventually triggered from the game thread tick
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLVisLegacyManager::Update, UpdateRate, true);
		}
		else
		{
			// Update logger on tick (updates every game thread tick, update rate can vary)
			SetComponentTickEnabled(true);
		}
		
		// Mark manager as started
		bIsStarted = true;
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	}
}

// Stop recording
void USLVisLegacyManager::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Stop update timer;
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}

		// Mark manager as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	}
}

// Called either from tick, or from the timer
void USLVisLegacyManager::Update()
{
	if (PixelFence.IsFenceComplete())
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Fence complete"), TEXT(__FUNCTION__), __LINE__);
		// Read the image data
		USLVisLegacyManager::ReadData();

		// Save the image data
		USLVisLegacyManager::SaveData();
	}

	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Fence NOT complete"), TEXT(__FUNCTION__), __LINE__);
}

// Read the image data
void USLVisLegacyManager::ReadData()
{
	/* Color */
	if (bCaptureColor)
	{
		FReadSurfaceDataFlags ReadSurfaceDataFlags;
		ReadSurfaceDataFlags.SetLinearToGamma(false);

		if (bCaptureColorFromViewport)
		{
			ColorViewport->Draw();
			//ColorViewport->ReadPixels(ColorImage);
			USLVisLegacyManager::ReadPixelsFromViewport(ColorImage);
		}
		else // Capture from scene capture component
		{
			FTextureRenderTargetResource* ColorRenderResource = ColorSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
			USLVisLegacyManager::ReadPixels(ColorRenderResource, ColorImage, ReadSurfaceDataFlags);
		}
		PixelFence.BeginFence();
	}

	/* Depth */
	if (bCaptureDepth)
	{
		FReadSurfaceDataFlags ReadSurfaceDataFlags;
		ReadSurfaceDataFlags.SetLinearToGamma(false);

		FTextureRenderTargetResource* DepthRenderResource = DepthSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
		USLVisLegacyManager::ReadPixels(DepthRenderResource, DepthImage, ReadSurfaceDataFlags);
		PixelFence.BeginFence();
	}

	/* Mask */
	if (bCaptureMask)
	{
		FReadSurfaceDataFlags ReadSurfaceDataFlags;
		ReadSurfaceDataFlags.SetLinearToGamma(false);

		FTextureRenderTargetResource* MaskRenderResource = MaskSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
		ReadPixels(MaskRenderResource, MaskImage, ReadSurfaceDataFlags);
		PixelFence.BeginFence();
	}

	/* Normal */
	if (bCaptureNormal)
	{
		FReadSurfaceDataFlags ReadSurfaceDataFlags;
		ReadSurfaceDataFlags.SetLinearToGamma(false);
		FTextureRenderTargetResource* NormalRenderResource = NormalSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
		ReadPixels(NormalRenderResource, NormalImage, ReadSurfaceDataFlags);
		PixelFence.BeginFence();
	}
}

// Read from viewport
void USLVisLegacyManager::ReadPixelsFromViewport(TArray<FColor>& OutImageData, FReadSurfaceDataFlags InFlags)
{
	FIntRect IntRect(0, 0, ColorViewport->GetSizeXY().X, ColorViewport->GetSizeXY().Y);

	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	// Clear previous data
	OutImageData.Reset();

	FReadSurfaceContext ReadSurfaceContext =
	{
		ColorViewport,
		&OutImageData,
		IntRect,
		InFlags,
	};

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		ReadSurfaceCommand,
		FReadSurfaceContext, Context, ReadSurfaceContext,
		{
			RHICmdList.ReadSurfaceData(
				Context.SrcRenderTarget->GetRenderTargetTexture(),
				Context.Rect,
				*Context.OutData,
				Context.Flags
			);
		});
}

// Read from scene capture component
void USLVisLegacyManager::ReadPixels(FTextureRenderTargetResource*& RenderResource, TArray<FColor>& OutImageData, FReadSurfaceDataFlags InFlags)
{
	FIntRect IntRect(0, 0, RenderResource->GetSizeXY().X, RenderResource->GetSizeXY().Y);

	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	// Clear previous data
	OutImageData.Reset();

	FReadSurfaceContext ReadSurfaceContext =
	{
		RenderResource,
		&OutImageData,
		IntRect,
		InFlags,
	};

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		ReadSurfaceCommand,
		FReadSurfaceContext, Context, ReadSurfaceContext,
		{
			RHICmdList.ReadSurfaceData(
				Context.SrcRenderTarget->GetRenderTargetTexture(),
				Context.Rect,
				*Context.OutData,
				Context.Flags
			);
		});
}

// Save the image data
void USLVisLegacyManager::SaveData()
{
	/* Color */
	if (bCaptureColor)
	{
		ImageWrapper->SetRaw(ColorImage.GetData(), ColorImage.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);

		TArray<uint8> CompressedData = ImageWrapper->GetCompressed();

		FString FullFilePath = FPaths::ProjectDir() + LogDirectory + TEXT("/Imgs/") +
			EpisodeId + TEXT("_color_") + FString::FromInt(ImgTickCount) + ".jpg";
		FPaths::RemoveDuplicateSlashes(FullFilePath);

		FFileHelper::SaveArrayToFile(CompressedData, *FullFilePath);
	}

	/* Depth */
	if (bCaptureDepth)
	{
		ImageWrapper->SetRaw(DepthImage.GetData(), DepthImage.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);

		TArray<uint8> CompressedData = ImageWrapper->GetCompressed();

		FString FullFilePath = FPaths::ProjectDir() + LogDirectory + TEXT("/Imgs/") +
			EpisodeId + TEXT("_depth_") + FString::FromInt(ImgTickCount) + ".jpg";
		FPaths::RemoveDuplicateSlashes(FullFilePath);

		FFileHelper::SaveArrayToFile(CompressedData, *FullFilePath);
	}

	/* Mask */
	if (bCaptureMask)
	{
		ImageWrapper->SetRaw(MaskImage.GetData(), MaskImage.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);

		TArray<uint8> CompressedData = ImageWrapper->GetCompressed();

		FString FullFilePath = FPaths::ProjectDir() + LogDirectory + TEXT("/Imgs/") +
			EpisodeId + TEXT("_mask_") + FString::FromInt(ImgTickCount) + ".jpg";
		FPaths::RemoveDuplicateSlashes(FullFilePath);

		FFileHelper::SaveArrayToFile(CompressedData, *FullFilePath);
	}

	/* Normal */
	if (bCaptureNormal)
	{
		ImageWrapper->SetRaw(NormalImage.GetData(), NormalImage.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);

		TArray<uint8> CompressedData = ImageWrapper->GetCompressed();

		FString FullFilePath = FPaths::ProjectDir() + LogDirectory + TEXT("/Imgs/") + 
			EpisodeId + TEXT("_normal_") + FString::FromInt(ImgTickCount) + ".jpg";
		FPaths::RemoveDuplicateSlashes(FullFilePath);

		FFileHelper::SaveArrayToFile(CompressedData, *FullFilePath);
	}
	++ImgTickCount;
}

// Create capture components
void USLVisLegacyManager::CreateCaptureComponents()
{	
	/* Color */
	USLVisLegacyManager::CreateColorCaptureComponent();

	/* Depth */
	USLVisLegacyManager::CreateDepthCaptureComponent();

	/* Mask */
	USLVisLegacyManager::CreateMaskCaptureComponent();

	/* Normal */
	USLVisLegacyManager::CreateNormalCaptureComponent();
}

// Create color capture component
void USLVisLegacyManager::CreateColorCaptureComponent()
{
	ColorSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ColorCapture"));
	ColorSceneCaptureComp->SetupAttachment(this);
	ColorSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	ColorSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("ColorTarget"));
	ColorSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	ColorSceneCaptureComp->FOVAngle = FOV;

	ColorSceneCaptureComp->SetHiddenInGame(true);
	ColorSceneCaptureComp->Deactivate();
}

// Create depth capture component
void USLVisLegacyManager::CreateDepthCaptureComponent()
{
	DepthSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("DepthCapture"));
	DepthSceneCaptureComp->SetupAttachment(this);
	DepthSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	DepthSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("DepthTarget"));
	DepthSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	DepthSceneCaptureComp->FOVAngle = FOV;

	//// Get depth scene material for post-processing
	//ConstructorHelpers::FObjectFinder<UMaterial> MaterialDepthFinder(TEXT("Material'/USemLog/M_SceneDepthWorldUnits.M_SceneDepthWorldUnits'"));
	//if (MaterialDepthFinder.Object != nullptr)
	//{
	//	//MaterialDepthInstance = UMaterialInstanceDynamic::Create(MaterialDepthFinder.Object, DepthSceneCaptureComp);
	//	MaterialDepthInstance = (UMaterial*)MaterialDepthFinder.Object;
	//	if (MaterialDepthInstance != nullptr)
	//	{
	//		// Store previous ShowFlags
	//		FEngineShowFlags PreviousShowFlags(DepthSceneCaptureComp->ShowFlags); 

	//		DepthSceneCaptureComp->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_All0);
	//		DepthSceneCaptureComp->ShowFlags.SetRendering(true);
	//		DepthSceneCaptureComp->ShowFlags.SetStaticMeshes(true);

	//		// Important for the correctness of tree leaves.
	//		DepthSceneCaptureComp->ShowFlags.SetMaterials(true);

	//		// These are minimal setting
	//		DepthSceneCaptureComp->ShowFlags.SetPostProcessing(true);
	//		DepthSceneCaptureComp->ShowFlags.SetPostProcessMaterial(true);

	//		// This option will change object material to vertex color material, which don't produce surface normal
	//		// ShowFlags.SetVertexColors(true);

	//		GVertexColorViewMode = EVertexColorViewMode::Color;

	//		// Store the visibility of the scene, such as folliage and landscape.
	//		DepthSceneCaptureComp->ShowFlags.SetStaticMeshes(PreviousShowFlags.StaticMeshes);
	//		DepthSceneCaptureComp->ShowFlags.SetLandscape(PreviousShowFlags.Landscape);

	//		DepthSceneCaptureComp->ShowFlags.SetInstancedFoliage(PreviousShowFlags.InstancedFoliage);
	//		DepthSceneCaptureComp->ShowFlags.SetInstancedGrass(PreviousShowFlags.InstancedGrass);
	//		DepthSceneCaptureComp->ShowFlags.SetInstancedStaticMeshes(PreviousShowFlags.InstancedStaticMeshes);

	//		DepthSceneCaptureComp->ShowFlags.SetSkeletalMeshes(PreviousShowFlags.SkeletalMeshes);

	//		DepthSceneCaptureComp->PostProcessSettings.AddBlendable(MaterialDepthInstance, 1);
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Could not load depth asset!"), TEXT(__FUNCTION__), __LINE__);
	//}

	DepthSceneCaptureComp->SetHiddenInGame(true);
	DepthSceneCaptureComp->Deactivate();
}

// Create mask capture component
void USLVisLegacyManager::CreateMaskCaptureComponent()
{
	MaskSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MaskCapture"));
	MaskSceneCaptureComp->SetupAttachment(this);
	MaskSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	MaskSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("MaskTarget"));
	MaskSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	MaskSceneCaptureComp->FOVAngle = FOV;

	FEngineShowFlags PreviousShowFlags(MaskSceneCaptureComp->ShowFlags); // Store previous ShowFlags
	ApplyViewMode(VMI_Lit, true, MaskSceneCaptureComp->ShowFlags);

	// From MeshPaintEdMode.cpp:2942
	MaskSceneCaptureComp->ShowFlags.SetMaterials(false);
	MaskSceneCaptureComp->ShowFlags.SetLighting(false);
	MaskSceneCaptureComp->ShowFlags.SetBSPTriangles(true);
	MaskSceneCaptureComp->ShowFlags.SetVertexColors(true);
	MaskSceneCaptureComp->ShowFlags.SetPostProcessing(false);
	MaskSceneCaptureComp->ShowFlags.SetHMDDistortion(false);
	MaskSceneCaptureComp->ShowFlags.SetTonemapper(false); // This won't take effect here

	GVertexColorViewMode = EVertexColorViewMode::Color;

	 // Store the visibility of the scene, such as folliage and landscape.
	MaskSceneCaptureComp->ShowFlags.SetStaticMeshes(PreviousShowFlags.StaticMeshes);
	MaskSceneCaptureComp->ShowFlags.SetLandscape(PreviousShowFlags.Landscape);

	MaskSceneCaptureComp->ShowFlags.SetInstancedFoliage(PreviousShowFlags.InstancedFoliage);
	MaskSceneCaptureComp->ShowFlags.SetInstancedGrass(PreviousShowFlags.InstancedGrass);
	MaskSceneCaptureComp->ShowFlags.SetInstancedStaticMeshes(PreviousShowFlags.InstancedStaticMeshes);

	MaskSceneCaptureComp->ShowFlags.SetSkeletalMeshes(PreviousShowFlags.SkeletalMeshes);

	MaskSceneCaptureComp->SetHiddenInGame(true);
	MaskSceneCaptureComp->Deactivate();
}

// Create normal capture component
void USLVisLegacyManager::CreateNormalCaptureComponent()
{
	NormalSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NormalCapture"));
	NormalSceneCaptureComp->SetupAttachment(this);
	NormalSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	NormalSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("NormalTarget"));
	NormalSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	NormalSceneCaptureComp->FOVAngle = FOV;

	//// Get Normal scene material for postprocessing
	//ConstructorHelpers::FObjectFinder<UMaterial> MaterialNormalFinder(TEXT("Material'/USemLog/M_WorldNormal.M_WorldNormal'"));
	//if (MaterialNormalFinder.Object != nullptr)
	//{
	//	MaterialNormalInstance = (UMaterial*)MaterialNormalFinder.Object;
	//	if (MaterialNormalInstance != nullptr)
	//	{
	//		FEngineShowFlags PreviousShowFlags(NormalSceneCaptureComp->ShowFlags); // Store previous ShowFlags
	//		
	//		NormalSceneCaptureComp->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_All0);
	//		NormalSceneCaptureComp->ShowFlags.SetRendering(true);
	//		NormalSceneCaptureComp->ShowFlags.SetStaticMeshes(true);
	//		// Important for the correctness of tree leaves.
	//		NormalSceneCaptureComp->ShowFlags.SetMaterials(true); 


	//		// These are minimal setting
	//		NormalSceneCaptureComp->ShowFlags.SetPostProcessing(true);
	//		NormalSceneCaptureComp->ShowFlags.SetPostProcessMaterial(true);
	//		// This option will change object material to vertex color material, which don't produce surface normal
	//		// ShowFlags.SetVertexColors(true); 

	//		GVertexColorViewMode = EVertexColorViewMode::Color;

	//		// Store the visibility of the scene, such as folliage and landscape.
	//		NormalSceneCaptureComp->ShowFlags.SetStaticMeshes(PreviousShowFlags.StaticMeshes);
	//		NormalSceneCaptureComp->ShowFlags.SetLandscape(PreviousShowFlags.Landscape);

	//		NormalSceneCaptureComp->ShowFlags.SetInstancedFoliage(PreviousShowFlags.InstancedFoliage);
	//		NormalSceneCaptureComp->ShowFlags.SetInstancedGrass(PreviousShowFlags.InstancedGrass);
	//		NormalSceneCaptureComp->ShowFlags.SetInstancedStaticMeshes(PreviousShowFlags.InstancedStaticMeshes);

	//		NormalSceneCaptureComp->ShowFlags.SetSkeletalMeshes(PreviousShowFlags.SkeletalMeshes);

	//		NormalSceneCaptureComp->PostProcessSettings.AddBlendable(MaterialNormalInstance, 1);
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Could not load normal asset!"), TEXT(__FUNCTION__), __LINE__);
	//}

	NormalSceneCaptureComp->SetHiddenInGame(true);
	NormalSceneCaptureComp->Deactivate();
}

// Init capture components
void USLVisLegacyManager::InitCaptureComponents()
{
	static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	// Read viewport resolution
	if (!bUseCustomResolution)
	{
		ColorViewport = GetWorld()->GetGameViewport()->Viewport;
		Width = ColorViewport->GetRenderTargetTextureSizeXY().X;
		Height = ColorViewport->GetRenderTargetTextureSizeXY().Y;
	}

	/* Color */
	if (bCaptureColor)
	{
		USLVisLegacyManager::InitColorCaptureComponent();
	}

	/* Depth */
	if (bCaptureDepth)
	{
		USLVisLegacyManager::InitDepthCaptureComponent();
	}

	/* Mask */
	if (bCaptureMask)
	{
		USLVisLegacyManager::InitMaskCaptureComponent();
	}

	/* Normal */
	if (bCaptureNormal)
	{
		USLVisLegacyManager::InitNormalCaptureComponent();
	}
}

// Init color capture component
void USLVisLegacyManager::InitColorCaptureComponent()
{
	ColorSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	ColorImage.AddZeroed(Width*Height);

	//ColorSceneCaptureComp->TextureTarget->TargetGamma = 1.4;
	ColorSceneCaptureComp->TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
	ColorSceneCaptureComp->SetHiddenInGame(false);
	ColorSceneCaptureComp->Activate();
}

// Init depth capture component
void USLVisLegacyManager::InitDepthCaptureComponent()
{
	DepthSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	DepthImage.AddZeroed(Width*Height);

	DepthSceneCaptureComp->TextureTarget->TargetGamma = 1.0;
	DepthSceneCaptureComp->SetHiddenInGame(false);
	DepthSceneCaptureComp->Activate();
}

// Init mask capture component
void USLVisLegacyManager::InitMaskCaptureComponent()
{
	// Create masks for the objects
	USLVisLegacyManager::InitMaskColors();


	MaskSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	MaskImage.AddZeroed(Width*Height);

	MaskSceneCaptureComp->TextureTarget->TargetGamma = 1.0;
	MaskSceneCaptureComp->SetHiddenInGame(false);
	MaskSceneCaptureComp->Activate();
}

// Init normal capture component
void USLVisLegacyManager::InitNormalCaptureComponent()
{
	NormalSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	NormalImage.AddZeroed(Width*Height);

	NormalSceneCaptureComp->TextureTarget->TargetGamma = 1.0;
	NormalSceneCaptureComp->SetHiddenInGame(false);
	NormalSceneCaptureComp->Activate();
}

// Create various colors for each object
void USLVisLegacyManager::InitMaskColors()
{
	TMap<AStaticMeshActor*, FColor> ActorColorMap;
	// All objects need to be selected in order for the mask to be active
	// see https://github.com/unrealcv/unrealcv/issues/98
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
		{
			GEditor->SelectActor(*ActItr, true, true);
			ActorColorMap.Add(*ActItr, FColor::Black);
		}

		// Create a unique color for each actor in world
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d WORLD"), TEXT(__FUNCTION__), __LINE__);
		USLVisLegacyManager::SetUniqueMaskColors(ActorColorMap);

		for (auto& ActorColorPair : ActorColorMap)
		{

		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d NO WORLD"), TEXT(__FUNCTION__), __LINE__);
	}


}

// Create an array of different colors
void USLVisLegacyManager::SetUniqueMaskColors(TMap<AStaticMeshActor*, FColor>& OutActorColorMap)
{
	const uint32 NrOfColors = OutActorColorMap.Num();
	const uint32 MaxHue = 50;
	// It shifts the next Hue value used, so that colors next to each other are not very similar.
	// this is only important for the human eye
	const uint32 ShiftHue = 11;
	const float MinSat = 0.65;
	const float MinVal = 0.65;
	
	uint32 HueCount = MaxHue;
	uint32 SatCount = 1;
	uint32 ValCount = 1;

	// Compute how many different Saturations and Values are needed
	int32_t left = FMath::Max<int32_t>(0, NrOfColors - HueCount);
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d left=%d"), TEXT(__FUNCTION__), __LINE__, left);
	while (left > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d left=%d"), TEXT(__FUNCTION__), __LINE__, left);
		if (left > 0)
		{
			++ValCount;
			left = NrOfColors - SatCount * ValCount * HueCount;
		}
		if (left > 0)
		{
			++SatCount;
			left = NrOfColors - SatCount * ValCount * HueCount;
		}
	}


	// Compute how many different saturations and values are needed
	int32 Remaining = FMath::Max<int32>(0, NrOfColors - HueCount);
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Remaining=%d, NRofCol=%d"),
		TEXT(__FUNCTION__), __LINE__, Remaining, NrOfColors);
	while (Remaining > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Remaining=%d, NRofCol=%d"),
			TEXT(__FUNCTION__), __LINE__, Remaining, NrOfColors);
		if (Remaining > 0)
		{
			++ValCount;
			Remaining = NrOfColors - SatCount * ValCount * HueCount;
		}
		if (Remaining > 0)
		{
			++SatCount;
			Remaining = NrOfColors - SatCount * ValCount * HueCount;
		}
	}

	const float StepHue = 360.0f / HueCount;
	const float StepSat = (1.0f - MinSat) / FMath::Max(1.0f, SatCount - 1.0f);
	const float StepVal = (1.0f - MinVal) / FMath::Max(1.0f, ValCount - 1.0f);

	TArray<FColor> Colors;
	Colors.Reserve(SatCount * ValCount * HueCount);

	FLinearColor HSVColor;
	for (uint32 S = 0; S < SatCount; ++S)
	{
		HSVColor.G = 1.0f - S * StepSat;
		for (uint32 V = 0; V < ValCount; ++V)
		{
			HSVColor.B = 1.0f - V * StepVal;
			for (uint32 H = 0; H < HueCount; ++H)
			{
				HSVColor.R = ((H * ShiftHue) % MaxHue) * StepHue;
				Colors.Add(HSVColor.HSVToLinearRGB().ToFColor(false));
			}
		}
	}

	// TODO merge these two steps, add the color directly to the map
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d NR cols: %d , NR Acts: %d"),
		TEXT(__FUNCTION__), __LINE__, Colors.Num(), OutActorColorMap.Num());
	if (Colors.Num() >= OutActorColorMap.Num())
	{
		for (auto& ActorColorPair : OutActorColorMap)
		{
			if (UStaticMeshComponent* SMComp = ActorColorPair.Key->GetStaticMeshComponent())
			{
				if (UStaticMesh* SM = SMComp->GetStaticMesh())
				{
					uint32 MeshLODIndex = 0;
					uint32 NumLODLevel = SM->RenderData->LODResources.Num();
					//check(NumLODLevel == 1);
					FStaticMeshLODResources &LODModel = SM->RenderData->LODResources[MeshLODIndex];
					SMComp->SetLODDataCount(1, SMComp->LODData.Num());
					FStaticMeshComponentLODInfo *InstanceMeshLODInfo = &SMComp->LODData[MeshLODIndex];

					// PaintingMeshLODIndex + 1 is the minimum requirement, enlarge if not satisfied

					InstanceMeshLODInfo->PaintedVertices.Empty();

					InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;

					InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(FColor::Green, LODModel.GetNumVertices());


					uint32 NumVertices = LODModel.GetNumVertices();
					//check(InstanceMeshLODInfo->OverrideVertexColors);
					//check(NumVertices <= InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices());


					for (uint32 ColorIndex = 0; ColorIndex < NumVertices; ColorIndex++)
					{
						//uint32 NumOverrideVertexColors = InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices();
						//uint32 NumPaintedVertices = InstanceMeshLODInfo->PaintedVertices.Num();
						InstanceMeshLODInfo->OverrideVertexColors->VertexColor(ColorIndex) = ActorColorPair.Value;
					}
					BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);

					SMComp->MarkRenderStateDirty();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Not engough colors for each actor, skipping mask setup."),
			TEXT(__FUNCTION__), __LINE__);
	}
}