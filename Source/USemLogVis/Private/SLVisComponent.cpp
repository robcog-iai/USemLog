// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisComponent.h"
#include "ConstructorHelpers.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"// todo rm, us SLMappings for iterating
#include "HAL/PlatformFilemanager.h"
#if WITH_EDITOR
#include "Components/ArrowComponent.h"
#endif // WITH_EDITOR

// Sets default values for this component's properties
USLVisComponent::USLVisComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

#if WITH_EDITOR
	// Location and orientation visualization of the component
	ArrowVis = CreateDefaultSubobject<UArrowComponent>(TEXT("SLVisArrowComponent"));
	ArrowVis->SetupAttachment(this);
	ArrowVis->ArrowSize = 1.5f;
	ArrowVis->ArrowColor = FColor::Blue;
#endif // WITH_EDITOR

	// Default parameters
	bUseCustomCameraId = false;
	CameraId = TEXT("autogen");
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

	LogDirectory = TEXT("SemLog/Img");
	EpisodeId = TEXT("EpId");

	// TODO rm
	bFirsttick = true;
	bColorSave = false;
	bDepthSave = false;
	bMaskSave = false;
	bNormalSave = false;

	bInitialAsyncTask = true;


	// Create the capture components
	ColorSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ColorCapture"));
	ColorSceneCaptureComp->SetupAttachment(this);
	ColorSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	ColorSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("ColorTarget"));
	ColorSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	ColorSceneCaptureComp->FOVAngle = FOV;

	DepthSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("DepthCapture"));
	DepthSceneCaptureComp->SetupAttachment(this);
	DepthSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	DepthSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("DepthTarget"));
	DepthSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	DepthSceneCaptureComp->FOVAngle = FOV;

	MaskSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MaskCapture"));
	MaskSceneCaptureComp->SetupAttachment(this);
	MaskSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	MaskSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("MaskTarget"));
	MaskSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	MaskSceneCaptureComp->FOVAngle = FOV;

	NormalSceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NormalCapture"));
	NormalSceneCaptureComp->SetupAttachment(this);
	NormalSceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	NormalSceneCaptureComp->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("NormalTarget"));
	NormalSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	NormalSceneCaptureComp->FOVAngle = FOV;

	// Get depth scene material for postprocessing
	ConstructorHelpers::FObjectFinder<UMaterial> MaterialDepthFinder(TEXT("Material'/USemLog/M_SceneDepthWorldUnits.M_SceneDepthWorldUnits'"));
	if (MaterialDepthFinder.Object != nullptr)
	{
		//MaterialDepthInstance = UMaterialInstanceDynamic::Create(MaterialDepthFinder.Object, DepthSceneCaptureComp);
		MaterialDepthInstance = (UMaterial*)MaterialDepthFinder.Object;
		if (MaterialDepthInstance != nullptr)
		{
			//PostProcess(DepthSceneCaptureComp->ShowFlags);
			PostProcess(DepthSceneCaptureComp->ShowFlags);
			DepthSceneCaptureComp->PostProcessSettings.AddBlendable(MaterialDepthInstance, 1);
			UE_LOG(LogTemp, Warning, TEXT("DEPTH LOADED"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not load material for depth."));
	}

	// Get Normal scene material for postprocessing
	ConstructorHelpers::FObjectFinder<UMaterial> MaterialNormalFinder(TEXT("Material'/USemLog/M_WorldNormal.M_WorldNormal'"));
	if (MaterialNormalFinder.Object != nullptr)
	{
		MaterialNormalInstance = (UMaterial*)MaterialNormalFinder.Object;
		if (MaterialNormalInstance != nullptr)
		{

			USLVisComponent::PostProcess(NormalSceneCaptureComp->ShowFlags);
			NormalSceneCaptureComp->PostProcessSettings.AddBlendable(MaterialNormalInstance, 1);
			UE_LOG(LogTemp, Warning, TEXT("NORMAL LOADED"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not load material for Normal."));
	}

	USLVisComponent::VertexColor(MaskSceneCaptureComp->ShowFlags);
	ColorSceneCaptureComp->SetHiddenInGame(true);
	ColorSceneCaptureComp->Deactivate();

	DepthSceneCaptureComp->SetHiddenInGame(true);
	DepthSceneCaptureComp->Deactivate();

	MaskSceneCaptureComp->SetHiddenInGame(true);
	MaskSceneCaptureComp->Deactivate();

	NormalSceneCaptureComp->SetHiddenInGame(true);
	NormalSceneCaptureComp->Deactivate();
}


// Called when the game starts
void USLVisComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

	// Init with a delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLVisComponent::Init, 1.0f, false);

}


// Called every frame
void USLVisComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

}

// Init component
void USLVisComponent::Init()
{
	static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	// Use viewport resolution
	if (!bUseCustomResolution)
	{
		ColorViewport = GetWorld()->GetGameViewport()->Viewport;
		Width = ColorViewport->GetRenderTargetTextureSizeXY().X;
		Height = ColorViewport->GetRenderTargetTextureSizeXY().Y;
	}

	ColorSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	// Initializing buffers for reading images from the GPU
	ColorImage.AddZeroed(Width*Height);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Image Size: x: %i, y: %i"), Width, Height));

	DepthSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	DepthImage.AddZeroed(Width*Height);

	MaskSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	MaskImage.AddZeroed(Width*Height);

	NormalSceneCaptureComp->TextureTarget->InitAutoFormat(Width, Height);
	NormalImage.AddZeroed(Width*Height);

	if (bCaptureColor) 
	{
		//ColorSceneCaptureComp->TextureTarget->TargetGamma = 1.4;
		ColorSceneCaptureComp->TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Gamma:%f"), GEngine->GetDisplayGamma()));
		ColorSceneCaptureComp->SetHiddenInGame(false);
		ColorSceneCaptureComp->Activate();
	}

	if (bCaptureDepth) 
	{
		DepthSceneCaptureComp->TextureTarget->TargetGamma = 1.0;
		DepthSceneCaptureComp->SetHiddenInGame(false);
		DepthSceneCaptureComp->Activate();
	}

	if (bCaptureMask) 
	{
		if (USLVisComponent::ColorAllObjects())
		{
			UE_LOG(LogTemp, Warning, TEXT("All the objects has colored"));
		}
		MaskSceneCaptureComp->TextureTarget->TargetGamma = 1.0;
		MaskSceneCaptureComp->SetHiddenInGame(false);
		MaskSceneCaptureComp->Activate();
		USLVisComponent::SetMaskColorFileHandle(LogDirectory);
	}

	if (bCaptureNormal) {
		NormalSceneCaptureComp->TextureTarget->TargetGamma = 1.0;
		NormalSceneCaptureComp->SetHiddenInGame(false);
		NormalSceneCaptureComp->Activate();
	}
	// Call the timer 
	USLVisComponent::SetFramerate(UpdateRate);
}

// TODO 
void USLVisComponent::SetFramerate(const float NewFramerate)
{
	if (NewFramerate > 0.0f)
	{
		// Update Camera on custom timer tick (does not guarantees the UpdateRate value,
		// since it will be eventually triggered from the game thread tick

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLVisComponent::TimerTick, 1 / NewFramerate, true);

	}
}

// Start capturing
void USLVisComponent::Start()
{

}

// Stop recording
void USLVisComponent::Finish()
{

}

// ViewMode for ground truth types implemented with PostProcess material
void USLVisComponent::PostProcess(FEngineShowFlags & ShowFlags)
{
	FEngineShowFlags PreviousShowFlags(ShowFlags); // Store previous ShowFlags
	USLVisComponent::BasicSetting(ShowFlags);
	// These are minimal setting
	ShowFlags.SetPostProcessing(true);
	ShowFlags.SetPostProcessMaterial(true);
	// ShowFlags.SetVertexColors(true); // This option will change object material to vertex color material, which don't produce surface normal

	GVertexColorViewMode = EVertexColorViewMode::Color;
	USLVisComponent::SetVisibility(ShowFlags, PreviousShowFlags); // Store the visibility of the scene, such as folliage and landscape.
}

void USLVisComponent::BasicSetting(FEngineShowFlags & ShowFlags)
{
	ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_All0);
	ShowFlags.SetRendering(true);
	ShowFlags.SetStaticMeshes(true);
	//ShowFlags.SetSkeletalMeshes(true); // TODO AnHa
	ShowFlags.SetMaterials(true); // Important for the correctness of tree leaves.
}

void USLVisComponent::SetVisibility(FEngineShowFlags & Target, FEngineShowFlags & Source)
{
	Target.SetStaticMeshes(Source.StaticMeshes);
	Target.SetLandscape(Source.Landscape);

	Target.SetInstancedFoliage(Source.InstancedFoliage);
	Target.SetInstancedGrass(Source.InstancedGrass);
	Target.SetInstancedStaticMeshes(Source.InstancedStaticMeshes);

	Target.SetSkeletalMeshes(Source.SkeletalMeshes);
}

void USLVisComponent::VertexColor(FEngineShowFlags & ShowFlags)
{
	FEngineShowFlags PreviousShowFlags(ShowFlags); // Store previous ShowFlags
	ApplyViewMode(VMI_Lit, true, ShowFlags);

	// From MeshPaintEdMode.cpp:2942
	ShowFlags.SetMaterials(false);
	ShowFlags.SetLighting(false);
	ShowFlags.SetBSPTriangles(true);
	ShowFlags.SetVertexColors(true);
	ShowFlags.SetPostProcessing(false);
	ShowFlags.SetHMDDistortion(false);
	ShowFlags.SetTonemapper(false); // This won't take effect here

	GVertexColorViewMode = EVertexColorViewMode::Color;
	SetVisibility(ShowFlags, PreviousShowFlags); // Store the visibility of the scene, such as folliage and landscape.
}

bool USLVisComponent::ColorAllObjects()
{
	uint32_t NumberOfActors = 0;

	for (TActorIterator<AStaticMeshActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{

		// Select all actors in editor, masking does not work without it
		// TODO wrap with IF_EDITOR
		FString ActorName = ActItr->GetName();
		GEditor->SelectActor(*ActItr, true, true);

		//FString CategoryName = ActorName.Left(7);
		if (!ObjectCategory.Contains(ActorName)) {
			++NumberOfActors;
			ObjectCategory.Add(ActorName);
		}

	}

	UE_LOG(LogTemp, Warning, TEXT("Found %d Actor Categories."), NumberOfActors);
	USLVisComponent::GenerateColors(NumberOfActors);

	return true;
}

void USLVisComponent::SetFileHandle(FString folder)
{
	const FString Filename = CameraId + "_" + folder + TEXT(".bson");
	FString EpisodesDirPath = FPaths::ProjectDir() + TEXT("/SemLog/") + CameraId + "/" + TEXT("/BsonFile/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);
	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath);
}

void USLVisComponent::SetMaskColorFileHandle(FString folder)
{
	const FString Filename = TEXT("Mask_Color_") + folder + TEXT(".bson");
	FString EpisodesDirPath = FPaths::ProjectDir() + TEXT("/SemLog/") + CameraId + "/" + TEXT("/MaskColor/");

	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);
	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	MaskColorFileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath);

}

void USLVisComponent::TimerTick()
{
	bool bSaveAsImage = true;
	FDateTime Stamp = FDateTime::UtcNow();
	if (!bFirsttick && PixelFence.IsFenceComplete())
	{
		if (bSaveAsImage)
		{
			if (bCaptureColor)
			{
				USLVisComponent::InitAsyncTask(ColorAsyncWorker, ColorImage, Stamp, LogDirectory, CameraId + TEXT("_COLOR_"), Width, Height);
				bColorSave = true;
			}

			if (bCaptureDepth)
			{
				USLVisComponent::InitAsyncTask(DepthAsyncWorker, DepthImage, Stamp, LogDirectory, CameraId + TEXT("_Depth_"), Width, Height);
				bDepthSave = true;
			}

			if (bCaptureMask)
			{
				USLVisComponent::InitAsyncTask(MaskAsyncWorker, MaskImage, Stamp, LogDirectory, CameraId + TEXT("_Mask_"), Width, Height);
				bMaskSave = true;
			}

			if (bCaptureNormal)
			{
				USLVisComponent::InitAsyncTask(NormalAsyncWorker, NormalImage, Stamp, LogDirectory, CameraId + TEXT("_Normal_"), Width, Height);
				bNormalSave = true;
			}
		}
	}
	if (bFirsttick)
	{
		if (bColorSave)
		{
			USLVisComponent::ProcessColorImg();
			bColorSave = false;
			UE_LOG(LogTemp, Warning, TEXT("Read Color Image"));
		}
		if (bDepthSave)
		{
			USLVisComponent::ProcessDepthImg();
			bDepthSave = false;
			UE_LOG(LogTemp, Warning, TEXT("Read Depth Image"));
		}

		if (bMaskSave)
		{
			USLVisComponent::ProcessMaskImg();
			bMaskSave = false;
			UE_LOG(LogTemp, Warning, TEXT("Read Mask Image"));
		}

		if (bNormalSave)
		{
			USLVisComponent::ProcessNormalImg();
			bNormalSave = false;
			UE_LOG(LogTemp, Warning, TEXT("Read Normal Image"));
		}
		bFirsttick = false;
	}
	if (!bFirsttick)
	{
		if (bColorSave)
		{
			USLVisComponent::ProcessColorImg();
			bColorSave = false;
		}
		if (bDepthSave)
		{
			USLVisComponent::ProcessDepthImg();
			bDepthSave = false;
		}
		if (bMaskSave)
		{
			USLVisComponent::ProcessMaskImg();
			bMaskSave = false;
		}
		if (bNormalSave)
		{
			USLVisComponent::ProcessNormalImg();
			bNormalSave = false;
		}
	}
}

void USLVisComponent::InitAsyncTask(FAsyncTask<RawDataAsyncWorker2>* AsyncWorker, TArray<FColor>& image, FDateTime Stamp, FString Folder, FString Name, int Width, int Height)
{

	AsyncWorker = new FAsyncTask<RawDataAsyncWorker2>(image, ImageWrapper, Stamp, Folder, Name, Width, Height, "MongoName", CameraId);
	AsyncWorker->StartBackgroundTask();
	AsyncWorker->EnsureCompletion();


}

void USLVisComponent::CurrentAsyncTask(FAsyncTask<RawDataAsyncWorker2>* AsyncWorker)
{
	if (AsyncWorker->IsDone() || bInitialAsyncTask)
	{
		bInitialAsyncTask = false;
		UE_LOG(LogTemp, Warning, TEXT("Start writing Color Image"));
		AsyncWorker->StartBackgroundTask();
		AsyncWorker->EnsureCompletion();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] SKIP new task "), TEXT(__FUNCTION__), __LINE__);
	}

}

void USLVisComponent::StopAsyncTask(FAsyncTask<RawDataAsyncWorker2>* AsyncWorker)
{
	if (AsyncWorker)
	{
		// Wait for worker to complete before deleting it
		AsyncWorker->EnsureCompletion();
		delete AsyncWorker;
		AsyncWorker = nullptr;
	}
}

void USLVisComponent::ProcessColorImg()
{
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);

	if (!bUseCustomResolution)
	{
		ColorViewport->Draw();
		//ColorViewport->ReadPixels(ColorImage);
		USLVisComponent::ReadPixels(ColorViewport, ColorImage);
	}
	if (bUseCustomResolution)
	{
		FTextureRenderTargetResource* ColorRenderResource = ColorSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
		USLVisComponent::ReadPixels(ColorRenderResource, ColorImage, ReadSurfaceDataFlags);
	}

	PixelFence.BeginFence();
}

void USLVisComponent::ProcessDepthImg()
{
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);
	FTextureRenderTargetResource* DepthRenderResource = DepthSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
	USLVisComponent::ReadPixels(DepthRenderResource, DepthImage, ReadSurfaceDataFlags);
	PixelFence.BeginFence();
}

void USLVisComponent::ProcessMaskImg()
{
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);

	FTextureRenderTargetResource* MaskRenderResource = MaskSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
	USLVisComponent::ReadPixels(MaskRenderResource, MaskImage, ReadSurfaceDataFlags);
	PixelFence.BeginFence();
}

void USLVisComponent::ProcessNormalImg()
{
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);
	FTextureRenderTargetResource* NormalRenderResource = NormalSceneCaptureComp->TextureTarget->GameThread_GetRenderTargetResource();
	USLVisComponent::ReadPixels(NormalRenderResource, NormalImage, ReadSurfaceDataFlags);
	PixelFence.BeginFence();
}

void USLVisComponent::ReadPixels(FViewport *& viewport, TArray<FColor>& OutImageData, FReadSurfaceDataFlags InFlags, FIntRect InRect)
{
	if (InRect == FIntRect(0, 0, 0, 0))
	{
		InRect = FIntRect(0, 0, viewport->GetSizeXY().X, viewport->GetSizeXY().Y);
	}

	// Read the render target surface data back.	
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	OutImageData.Reset();
	FReadSurfaceContext ReadSurfaceContext =
	{
		viewport,
		&OutImageData,
		InRect,
		InFlags
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



void USLVisComponent::ReadPixels(FTextureRenderTargetResource *& RenderResource, TArray<FColor>& OutImageData, FReadSurfaceDataFlags InFlags, FIntRect InRect)
{

	// Read the render target surface data back.	
	if (InRect == FIntRect(0, 0, 0, 0))
	{
		InRect = FIntRect(0, 0, RenderResource->GetSizeXY().X, RenderResource->GetSizeXY().Y);
	}
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	OutImageData.Reset();
	FReadSurfaceContext ReadSurfaceContext =
	{
		RenderResource,
		&OutImageData,
		InRect,
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

bool USLVisComponent::ColorObject(AActor * Actor, const FString & name)
{
	const FColor &ObjectColor = ObjectColors[ObjectToColor[name]];
	TArray<UMeshComponent *> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);
	for (auto MeshComponent : PaintableComponents)

	{
		if (MeshComponent == nullptr)
			continue;

		if (UStaticMeshComponent *StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			if (UStaticMesh *StaticMesh = StaticMeshComponent->GetStaticMesh())
			{
				uint32 PaintingMeshLODIndex = 0;
				uint32 NumLODLevel = StaticMesh->RenderData->LODResources.Num();
				//check(NumLODLevel == 1);
				FStaticMeshLODResources &LODModel = StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
				StaticMeshComponent->SetLODDataCount(1, StaticMeshComponent->LODData.Num());
				FStaticMeshComponentLODInfo *InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];

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
					InstanceMeshLODInfo->OverrideVertexColors->VertexColor(ColorIndex) = ObjectColor;
				}
				BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);

				StaticMeshComponent->MarkRenderStateDirty();

				//UE_LOG(LogTemp, Warning, TEXT("%s:%s has %d vertices,%d,%d,%d"), *Actor->GetActorLabel(), *StaticMeshComponent->GetName(), NumVertices, InstanceMeshLODInfo->OverrideVertexColors->VertexColor(0).R, InstanceMeshLODInfo->OverrideVertexColors->VertexColor(0).G, InstanceMeshLODInfo->OverrideVertexColors->VertexColor(0).B)
			}
		}
	}
	return true;
}

void USLVisComponent::GenerateColors(const uint32_t NumberOfColors)
{
	const int32_t MaxHue = 50;
	// It shifts the next Hue value used, so that colors next to each other are not very similar. This is just important for humans
	const int32_t ShiftHue = 11;
	const float MinSat = 0.65;
	const float MinVal = 0.65;

	uint32_t HueCount = MaxHue;
	uint32_t SatCount = 1;
	uint32_t ValCount = 1;

	// Compute how many different Saturations and Values are needed
	int32_t left = FMath::Max<int32_t>(0, NumberOfColors - HueCount);
	while (left > 0)
	{
		if (left > 0)
		{
			++ValCount;
			left = NumberOfColors - SatCount * ValCount * HueCount;
		}
		if (left > 0)
		{
			++SatCount;
			left = NumberOfColors - SatCount * ValCount * HueCount;
		}
	}

	const float StepHue = 360.0f / HueCount;
	const float StepSat = (1.0f - MinSat) / FMath::Max(1.0f, SatCount - 1.0f);
	const float StepVal = (1.0f - MinVal) / FMath::Max(1.0f, ValCount - 1.0f);

	ObjectColors.Reserve(SatCount * ValCount * HueCount);
	UE_LOG(LogTemp, Warning, TEXT("Generating %d colors."), SatCount * ValCount * HueCount);

	FLinearColor HSVColor;
	for (uint32_t s = 0; s < SatCount; ++s)
	{
		HSVColor.G = 1.0f - s * StepSat;
		for (uint32_t v = 0; v < ValCount; ++v)
		{
			HSVColor.B = 1.0f - v * StepVal;
			for (uint32_t h = 0; h < HueCount; ++h)
			{
				HSVColor.R = ((h * ShiftHue) % MaxHue) * StepHue;
				ObjectColors.Add(HSVColor.HSVToLinearRGB().ToFColor(false));
				UE_LOG(LogTemp, Warning, TEXT("Added color %d: %d %d %d"), ObjectColors.Num(), ObjectColors.Last().R, ObjectColors.Last().G, ObjectColors.Last().B);
			}
		}
	}
}
