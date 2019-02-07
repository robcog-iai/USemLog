// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLoggerSpectatorPC.h"
#include "Engine/DemoNetDriver.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "SLVisCameraView.h"
#include "SLVisMaskHelper.h"
#include "SLVisImageWriterMongoCxx.h"
#include "SLVisImageWriterMongoC.h"
#include "SLVisImageWriterFile.h"

#include "Tags.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
ASLVisLoggerSpectatorPC::ASLVisLoggerSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	DemoUpdateRate = 0.28f;
	NewEntryTimeRange = 0.22f; 
	ActiveCameraViewIndex = 0;
	ActiveViewTypeIndex = 0;
	DemoTimestamp = 0.f;
	NumImagesProcessed = 0;
	NumImagesToProcess = 0;
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	
	bMaskMaterialsOn = false;
	//bMaskInit = false;

	// Add buffer types to visualize
	ViewTypes.Add(NAME_None); // Default will be color
	////ViewTypes.Add("SceneDepth");
	////ViewTypes.Add("WorldNormal");
	//ViewTypes.Add("SLSceneDepth");
	ViewTypes.Add("SLSceneDepthWorldUnits");
	ViewTypes.Add("SLMask");
	
	//if (ViewTypes.Contains(FName("SLMask")))
	//{
	//	static ConstructorHelpers::FObjectFinder<UMaterial> DMM(
	//		TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
	//	if (DMM.Succeeded())
	//	{
	//		MaskVisHelper = MakeShareable(new FSLVisMaskHelper(DMM.Object));
	//	}
	//	else
	//	{
	//		ViewTypes.Remove(FName("SLMask"));
	//	}
	//}

	// Image size
	//ResX = 640;
	//ResY = 480;
	//// 8k
	//ResX = 7680;
	//ResY = 4320;
	//// 4k
	ResX = 3840;
	ResY = 2160;
	//// 2k
	//ResX = 2048;
	//ResY = 1080;
	//// fhd
	//ResX = 1920;
	//ResY = 1080;
	//// hd
	//ResX = 1280;
	//ResY = 720;
	//// sd
	//ResX = 720;
	//ResY = 480;


	static ConstructorHelpers::FObjectFinder<UMaterial> DMM(
		TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
	if (DMM.Succeeded())
	{
		DefaultMaskMaterial = DMM.Object;
		DefaultMaskMaterial->bUsedWithStaticLighting = true;
	}
	bMaskMaterialsOn = false;
}

// Called when the game starts or when spawned
void ASLVisLoggerSpectatorPC::BeginPlay()
{
	Super::BeginPlay();

	// Start logger
	ASLVisLoggerSpectatorPC::MaskTimerInit();

	if (GetWorld()->DemoNetDriver && GetWorld()->DemoNetDriver->IsPlaying())
	{
		// Start paused
		ASLVisLoggerSpectatorPC::DemoPause();

		// Init logger
		ASLVisLoggerSpectatorPC::Init();

		// Start logger
		ASLVisLoggerSpectatorPC::Start();
	}
}

// Called when actor removed from game or game ended
void ASLVisLoggerSpectatorPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Finish logger
	//ASLVisLoggerSpectatorPC::Finish();
}

// Called to bind functionality to input
void ASLVisLoggerSpectatorPC::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// Called every frame
 void ASLVisLoggerSpectatorPC::Tick(float DeltaTime)
 {
	 Super::Tick(DeltaTime);
 }

// Init logger
void ASLVisLoggerSpectatorPC::Init()
{
	if (!bIsInit)
	{
		ActiveCameraViewIndex = 0;
		ActiveViewTypeIndex = 0;
		DemoTimestamp = 0.f;
		NumImagesProcessed = 0;
		NumImagesToProcess = 0;

		if (MaskVisHelper)
		{
			MaskVisHelper->Init(this);
		}

		// Make sure the time offset is not larger than the replay update rate
		FMath::Clamp(NewEntryTimeRange, 0.f, DemoUpdateRate);

		// Continue only if driver and viewport is available
		NetDriver = GetWorld()->DemoNetDriver;
		ViewportClient = GetWorld()->GetGameViewport();
		if (NetDriver && ViewportClient)
		{
			// Get episode id by removing suffix
			FString EpisodeId = NetDriver->GetActiveReplayName();
			EpisodeId.RemoveFromEnd("_RP");

			// Create writer
#if SLVIS_WITH_LIBMONGO
			//Writer = NewObject<USLVisImageWriterMongoC>(this);
			//Writer->Init(FSLVisImageWriterParams(
			//	TEXT("SemLog"), EpisodeId, NewEntryTimeRange, "127.0.0.1", 27017));

			//Writer = NewObject<USLVisImageWriterMongoCxx>(this);
			//Writer->Init(FSLVisImageWriterParams(
			//	TEXT("SemLog"), EpisodeId, "127.0.0.1", 27017));

			Writer = NewObject<USLVisImageWriterFile>(this);
			Writer->Init(FSLVisImageWriterParams(
				FPaths::ProjectDir() + TEXT("/SemLog/Episodes/"), EpisodeId));
#else
			Writer = NewObject<USLVisImageWriterFile>(this);
			Writer->Init(FSLVisImageWriterParams(
				FPaths::ProjectDir() + TEXT("/SemLog/Episodes/"), EpisodeId));
#endif //SLVIS_WITH_LIBMONGO

#if WITH_EDITOR
			//ProgressBar = MakeUnique<FScopedSlowTask>(NumImagesToSave, FText::FromString(FString(TEXT("Saving images.."))));
			//ProgressBar->MakeDialog(true, true);
#endif //WITH_EDITOR

			// Flag as initialized
			if (Writer && Writer->IsInit())
			{
				// Get camera views from the world
				for (TActorIterator<ASLVisCameraView>Itr(GetWorld()); Itr; ++Itr)
				{
					CameraViews.Add(*Itr);
				}
				// Calculate the total images to be saved
				uint32 NumTimesteps = (uint32)(NetDriver->DemoTotalTime / DemoUpdateRate) + 1;
				uint32 NumImages = CameraViews.Num() * ViewTypes.Num();
				NumImagesToProcess = NumTimesteps * NumImages;

				// Check which timestamp should be rendered first
				while (DemoTimestamp < NetDriver->DemoTotalTime &&
					ASLVisLoggerSpectatorPC::ShouldSkipThisFrame(DemoTimestamp))
				{
					// Update the number of saved images (even if timestamps are skipped, for tracking purposes)
					NumImagesProcessed += CurrImagesData.Num();
					DemoTimestamp += DemoUpdateRate;
				}

				// Set rendering parameters
				ASLVisLoggerSpectatorPC::SetupRenderingProperties();

				// Bind callback functions
				NetDriver->OnGotoTimeDelegate.AddUObject(this, &ASLVisLoggerSpectatorPC::DemoGotoCB);
				NetDriver->OnDemoFinishPlaybackDelegate.AddUObject(this, &ASLVisLoggerSpectatorPC::Finish);
				ViewportClient->OnScreenshotCaptured().AddUObject(this, &ASLVisLoggerSpectatorPC::ScreenshotCB);
				bIsInit = true;
			}
		}
	}
}

// Start logger
void ASLVisLoggerSpectatorPC::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Set camera to first target and first rendering type
		if (ASLVisLoggerSpectatorPC::SetFirstViewTarget() && ASLVisLoggerSpectatorPC::SetFirstViewType())
		{
			// Go to the beginning of the demo and start requesting screenshots
			NetDriver->GotoTimeInSeconds(DemoTimestamp);

			// Flag as started
			bIsStarted = true;
		}
	}
}

// Finish logger
void ASLVisLoggerSpectatorPC::Finish()
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		if (Writer)
		{
			Writer->Finish();
		}

		UE_LOG(LogTemp, Warning, TEXT("%s::%d Finished! Time=[%f/%f] Progress=[%d/%d]"),
			TEXT(__FUNCTION__), __LINE__,
			DemoTimestamp, NetDriver->DemoTotalTime,
			NumImagesProcessed, NumImagesToProcess);

		// Flag as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	
		// Crashes if called from EndPlay
		// Try to quit editor
		ASLVisLoggerSpectatorPC::QuitEditor();
	}
}

// Set rendered image quality
void ASLVisLoggerSpectatorPC::SetupRenderingProperties()
{	
	// TODO this probably causes an extra screenshot callback
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(ResX, ResY, 1.0f);
	// Set res sets this to true, which triggers the callback, we set it back to false
	GIsHighResScreenshot = false;

	// TODO see if this is useful
	//GetHighResScreenshotConfig().ParseConsoleCommand();
	
	// TODO this does not seem to have an effect
	// Set the display resolution for the current game view. Has no effect in the editor
	// e.g. 1280x720w for windowed, 1920x1080f for fullscreen, 1920x1080wf for windowed fullscreen
	//FString ResStr = FString::FromInt(ResX) + "x" + FString::FromInt(ResY)/* + "f"*/;
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetRes"))->Set(*ResStr);

	// Which anti-aliasing mode is used by default
	// 	AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(1);

	// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// Defines the postprocess anti aliasing method which allows to adjust for quality or performance. 
	// 0:off, 1:very low (faster FXAA), 2:low (FXAA), 3:medium (faster TemporalAA), 4:high (default TemporalAA), 5:very high, 6:max
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"))->Set(6);

	// Defines the motion blur method which allows to adjust for quality or performance
	// 0:off, 1:low, 2:medium, 3:high (default), 4: very high
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))->Set(0);

	// Whether to use screen space reflections and at what quality setting. 
	// (limits the setting in the post process settings which has a different scale)
	// (costs performance, adds more visual realism but the technique has limits)
	// 0: off (default), 1: low (no glossy), 2: medium (no glossy), 3: high (glossy/using roughness, few samples), 4: very high (likely too slow for real-time)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.SSR.Quality"))->Set(4);

	// Defines the eye adaptation quality which allows to adjust for quality or performance. 
	// 0: off (fastest), 1: low quality (e.g. non histogram based, not yet implemented), 2: normal quality (default), 3: high quality (e.g. screen position localized, not yet implemented)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.EyeAdaptationQuality"))->Set(0);

	// Improve the visual depth and fidelity of the scene because of a more accurate approximation of shadowing
	// 0: disabled, 1: enabled
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ContactShadows"))->Set(1);

	// This will adjust the luminance intensity of the image to accurately reproduce colors. Raising or lowering this value will result in the image mid-tones being washed-out or too dark. 
	// Default 0.0f;
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.TonemapperGamma"))->Set(2.2f);

	// Force highest LOD
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Called after a successful scrub
void ASLVisLoggerSpectatorPC::DemoGotoCB()
{
	// Pause the replay
	ASLVisLoggerSpectatorPC::DemoPause();

	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		GetHighResScreenshotConfig().FilenameOverride = ISLVisImageWriterInterface::CreateImageFilename(DemoTimestamp,
			CameraViews[ActiveCameraViewIndex]->GetCameraLabel(), ViewTypes[ActiveViewTypeIndex]);
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when screenshot is captured
void ASLVisLoggerSpectatorPC::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	// Make sure that all alpha values are opaque.
	TArray<FColor>& BitmapRef = const_cast<TArray<FColor>&>(Bitmap);
	for (auto& Color : BitmapRef)
	{
		Color.A = 255;
	}

	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, BitmapRef, CompressedBitmap);

	// Add image to the array in this timeslice
	CurrImagesData.Emplace(FSLVisImageData(FSLVisImageMetadata(ViewTypes[ActiveViewTypeIndex],
				CameraViews[ActiveCameraViewIndex]->GetCameraLabel(), ResX, ResY, DemoTimestamp), CompressedBitmap));

#if WITH_EDITOR
	//ProgressBar->EnterProgressFrame();
#endif //WITH_EDITOR

	// Check if multiple view types are required
	if (ASLVisLoggerSpectatorPC::SetNextViewType())
	{
		// Request screenshot on game thread
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			GetHighResScreenshotConfig().FilenameOverride = ISLVisImageWriterInterface::CreateImageFilename(DemoTimestamp,
				CameraViews[ActiveCameraViewIndex]->GetCameraLabel(), ViewTypes[ActiveViewTypeIndex]);
			ViewportClient->Viewport->TakeHighResScreenShot();
		});
	}
	else
	{
		// Go back to the first view type
		if (ASLVisLoggerSpectatorPC::SetFirstViewType())
		{
			// Check if more views are available
			if (ASLVisLoggerSpectatorPC::SetNextViewTarget())
			{
				// Request screenshot on game thread
				AsyncTask(ENamedThreads::GameThread,[this]() 
				{
					GetHighResScreenshotConfig().FilenameOverride = ISLVisImageWriterInterface::CreateImageFilename(DemoTimestamp,
						CameraViews[ActiveCameraViewIndex]->GetCameraLabel(), ViewTypes[ActiveViewTypeIndex]);
					ViewportClient->Viewport->TakeHighResScreenShot();
				});
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Sending %d images.. Time=[%f/%f] Progress=[%d/%d]"),
					TEXT(__FUNCTION__), __LINE__, CurrImagesData.Num(),
					DemoTimestamp, NetDriver->DemoTotalTime,
					NumImagesProcessed, NumImagesToProcess);

				// We reached the last img (camera + view) in this time slice, write data
				Writer->Write(DemoTimestamp, CurrImagesData);

				// Check for next timeslice that should be rendered (only for mongo writers)
				do {
					// Update the number of saved images (even if timestamps are skipped, for tracking purposes)
					NumImagesProcessed += CurrImagesData.Num();
					DemoTimestamp += DemoUpdateRate;
				} while (DemoTimestamp < NetDriver->DemoTotalTime &&
					ASLVisLoggerSpectatorPC::ShouldSkipThisFrame(DemoTimestamp));

				// Clear previous data array
				CurrImagesData.Empty();

				// Go back to first view
				if (ASLVisLoggerSpectatorPC::SetFirstViewTarget())
				{
					// Unpause demo in order to scrub
					ASLVisLoggerSpectatorPC::DemoUnPause();
					NetDriver->GotoTimeInSeconds(DemoTimestamp);
				}
			}
		}
	}
}

// Called when the demo reaches the last frame
void ASLVisLoggerSpectatorPC::QuitEditor()
{
	//FGenericPlatformMisc::RequestExit(false);
	//
	//FGameDelegates::Get().GetExitCommandDelegate().Broadcast();
	//FPlatformMisc::RequestExit(0);

	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));

	}
}

// Pause demo
void ASLVisLoggerSpectatorPC::DemoPause()
{
	// else, it is already paused
	if (GetWorld()->GetWorldSettings()->Pauser == nullptr)
	{
		if (GetWorld()->DemoNetDriver != nullptr &&
			GetWorld()->DemoNetDriver->ServerConnection != nullptr &&
			GetWorld()->DemoNetDriver->ServerConnection->OwningActor != nullptr)
		{
			GetWorld()->GetWorldSettings()->Pauser = PlayerState;
		}
	}
}

// Un-pause demo
void ASLVisLoggerSpectatorPC::DemoUnPause()
{
	// else, it is already un-paused
	if (GetWorld()->GetWorldSettings()->Pauser != nullptr)
	{
		GetWorld()->GetWorldSettings()->Pauser = nullptr;
	}
}

// Check if demo is paused
bool ASLVisLoggerSpectatorPC::IsDemoPaused()
{
	// Demo is paused if no player state is set as the Pauser
	return GetWorld()->GetWorldSettings()->Pauser == nullptr;
}

// Sets the first view, returns false if there are no views at all
bool ASLVisLoggerSpectatorPC::SetFirstViewTarget()
{
	ActiveCameraViewIndex = 0;
	if (CameraViews.IsValidIndex(ActiveCameraViewIndex))
	{
		SetViewTarget(CameraViews[ActiveCameraViewIndex], FViewTargetTransitionParams());
		return true;
	}
	else
	{
		// 0 is not a valid index, i.e. array is empty
		return false;
	}
}

// Sets the next view, returns false there are no more views
bool ASLVisLoggerSpectatorPC::SetNextViewTarget()
{
	ActiveCameraViewIndex++;
	if (CameraViews.IsValidIndex(ActiveCameraViewIndex))
	{
		SetViewTarget(CameraViews[ActiveCameraViewIndex], FViewTargetTransitionParams());
		return true;
	}
	else
	{
		return false;
	}
}

// Sets the first visualization buffer type, false if none
bool ASLVisLoggerSpectatorPC::SetFirstViewType()
{
	ActiveViewTypeIndex = 0;
	if (ViewTypes.IsValidIndex(ActiveViewTypeIndex))
	{
		return ASLVisLoggerSpectatorPC::ChangeViewType(ViewTypes[ActiveViewTypeIndex]);
	}
	else
	{
		// 0 is not a valid index, i.e. array is empty
		return false;
	}
}

// Sets the next visualization buffer type, returns false there are no more views
bool ASLVisLoggerSpectatorPC::SetNextViewType()
{
	ActiveViewTypeIndex++;
	if (ViewTypes.IsValidIndex(ActiveViewTypeIndex))
	{
		return ASLVisLoggerSpectatorPC::ChangeViewType(ViewTypes[ActiveViewTypeIndex]);
	}
	else
	{
		return false;
	}
}

// Render the given view type
bool ASLVisLoggerSpectatorPC::ChangeViewType(const FName& ViewType)
{
	// Get the console variable for switching buffer views
	static IConsoleVariable* ICVarVisTarget = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));
	
	// Switch between buffer visualization and its types
	if (ICVarVisTarget)
	{
		if (ViewportClient->GetEngineShowFlags())
		{
			if (ViewType.IsEqual(FName("SLMask")))
			{
				if (true/*&& MaskVisHelper && MaskVisHelper->ApplyMaskMaterials()*/)
				{
					ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
					//ViewportClient->GetEngineShowFlags()->SetTonemapper(true);
					ICVarVisTarget->Set(TEXT("BaseColor"));

					ASLVisLoggerSpectatorPC::SetMaskMaterials();
					bMaskMaterialsOn = true;
				}
			}
			else
			{
				if (bMaskMaterialsOn /*&& MaskVisHelper*/ /*&& MaskVisHelper->ApplyOriginalMaterials()*/)
				{
					ASLVisLoggerSpectatorPC::SetOrigMaterials();
					bMaskMaterialsOn = false;
				}
				ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(ViewType == NAME_None ? false : true);
				//ViewportClient->GetEngineShowFlags()->SetTonemapper(ViewType == NAME_None ? false : true);
				ICVarVisTarget->Set(*ViewType.ToString());
			}
			return true;
		}
	}
	return false;
}

// Skip the current timestamp (return false if not a mongo writer)
bool ASLVisLoggerSpectatorPC::ShouldSkipThisFrame(float Timestamp)
{
	if (Writer && Writer->IsInit())
	{
		// Check writer type
		if (USLVisImageWriterMongoCxx* AsMongoCxxWriter = Cast<USLVisImageWriterMongoCxx>(Writer.GetObject()))
		{
			if (AsMongoCxxWriter->ShouldSkipThisTimestamp(Timestamp))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Skipping the current frame images.. Time=[%f/%f] Progress=[%d/%d]"),
					TEXT(__FUNCTION__), __LINE__,
					DemoTimestamp, NetDriver->DemoTotalTime,
					NumImagesProcessed, NumImagesToProcess);
				return true;
			}
			else
			{
				return false;
			}
			//return AsMongoCxxWriter->ShouldSkipThisTimestamp(Timestamp);
		}
		else if (USLVisImageWriterMongoC* AsMongoCWriter = Cast<USLVisImageWriterMongoC>(Writer.GetObject()))
		{
			if (AsMongoCWriter->ShouldSkipThisFrame(Timestamp))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Skipping the current frame images.. Time=[%f/%f] Progress=[%d/%d]"),
					TEXT(__FUNCTION__), __LINE__,
					DemoTimestamp, NetDriver->DemoTotalTime,
					NumImagesProcessed, NumImagesToProcess);
				return true;
			}
			else
			{
				return false;
			}
			//return AsMongoCWriter->ShouldSkipThisTimestamp(Timestamp);
		}

		// For other wirters don't skip anything
		return false;
	}

	// No valid writer, skip frames
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Skipping %d images.. Time=[%f/%f] Progress=[%d/%d]"),
		TEXT(__FUNCTION__), __LINE__, CurrImagesData.Num(),
		DemoTimestamp, NetDriver->DemoTotalTime,
		NumImagesProcessed, NumImagesToProcess);
	return true;
}

/////////////////////////////////////////////
void ASLVisLoggerSpectatorPC::MaskTimerInit()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Iterating SMAs"), TEXT(__FUNCTION__), __LINE__);

	// Iterate static meshes from the world
	for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
	{
		if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
		{
			// Cache original materials
			MeshesOrigMaterials.Add(SMC, SMC->GetMaterials());

			FString ColorHex = FTags::GetValue(*SMAItr, "SemLog", "VisMask");
			if (!ColorHex.IsEmpty())
			{
				UMaterialInstanceDynamic* DynamicMaskMat = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
				DynamicMaskMat->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(FColor::FromHex(ColorHex)));
				MeshesMaskMaterials.Emplace(SMC, DynamicMaskMat);
			}
			else
			{
				ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					UMaterialInstanceDynamic* DynamicMaskMat = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMat->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(FColor::FromHex(ColorHex)));
					MeshesMaskMaterials.Emplace(SMC, DynamicMaskMat);
				}
				else
				{
					UnknownMeshes.Emplace(SMC);
				}
			}
		}
	}

	// TODO include semantic data from skeletal meshes as well
	// Iterate skeletal meshes
	for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
	{
		if (USkeletalMeshComponent* SkMC = SkMAItr->GetSkeletalMeshComponent())
		{
			// Add original materials
			MeshesOrigMaterials.Add(SkMC, SkMC->GetMaterials());

			// Check if the entity is annotated with a semantic color mask
			// TODO how to read bone colors
			FString ColorHex = FTags::GetValue(*SkMAItr, "SemLog", "VisMask");
			if (!ColorHex.IsEmpty())
			{
				//AddDynamicMaskMaterialsLambda(SkMC, ColorHex, InParent);
			}
			else
			{
				ColorHex = FTags::GetValue(SkMC, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					//AddDynamicMaskMaterialsLambda(SkMC, ColorHex, InParent);
				}
				else
				{
					UnknownMeshes.Emplace(SkMC);
				}
			}
		}
	}

	DynamicDefaultMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
	DynamicDefaultMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::Black);
	//GetWorldTimerManager().SetTimer(MaterialSwitchTimerHandle, this, &ASLVisLoggerSpectatorPC::TimerSwitchMaterials, 1.f, true, 1.f);
}

void ASLVisLoggerSpectatorPC::TimerSwitchMaterials()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Timer switch"), TEXT(__FUNCTION__), __LINE__);
	if (!bMaskMaterialsOn)
	{
		// Put original materials back
		ASLVisLoggerSpectatorPC::SetMaskMaterials();
		bMaskMaterialsOn = true;
	}
	else
	{
		// Put mask materials on
		ASLVisLoggerSpectatorPC::SetOrigMaterials();
		bMaskMaterialsOn = false;
	}
}

void ASLVisLoggerSpectatorPC::SetMaskMaterials()
{
	UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Setting mask materials.."), TEXT(__FUNCTION__), __LINE__);
	for (auto& SMCMat : MeshesMaskMaterials)
	{
		
		for (int32 MatIdx = 0; MatIdx < SMCMat.Key->GetNumMaterials(); ++MatIdx)
		{
			SMCMat.Key->SetMaterial(MatIdx, SMCMat.Value);
		}
	}

	for (auto& Mesh : UnknownMeshes)
	{
		for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
		{
			Mesh->SetMaterial(Idx, DefaultMaskMaterial);
			//Mesh->SetMaterial(Idx, DynamicDefaultMaskMaterial);
		}
	}
}

void ASLVisLoggerSpectatorPC::SetOrigMaterials()
{
	UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Setting original materials.."), TEXT(__FUNCTION__), __LINE__);
	for (auto& SMCMat : MeshesOrigMaterials)
	{
		int32 MatIdx = 0;
		for (auto& OrigMat : SMCMat.Value)
		{
			SMCMat.Key->SetMaterial(MatIdx, OrigMat);
			UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Set Idx=%d set"), TEXT(__FUNCTION__), __LINE__, MatIdx);
			MatIdx++;
		}
	}
}

// Creates a new unique mask color
FLinearColor ASLVisLoggerSpectatorPC::CreateNewMaskColor()
{
	FLinearColor RandColor = FLinearColor::MakeRandomColor();
	//RandColor.ToFColor(true).ToHex();

	// Add first color
	if (MaskColors.Num() == 0)
	{
		MaskColors.Emplace(RandColor);
		UE_LOG(LogTemp, Warning, TEXT("\t\t\t%s::%d Add first color %s"), TEXT(__FUNCTION__), __LINE__, *RandColor.ToString());
		return RandColor;
	}
	else
	{
		int32 NrOfTrials = 0;
		while (MaskColors.AddUnique(RandColor) == INDEX_NONE)
		{
			RandColor = FLinearColor::MakeRandomColor();
			NrOfTrials++;
			if (NrOfTrials > 100)
			{
				UE_LOG(LogTemp, Error, TEXT("\t\t\t\t%s::%d Could not generate a new random color (after 100 trials).."), TEXT(__FUNCTION__), __LINE__);
				return FLinearColor(ForceInit);
			}
			UE_LOG(LogTemp, Error, TEXT("\t\t\t\t%s::%d Clash at trial=%d with Col=%s"), TEXT(__FUNCTION__), __LINE__, NrOfTrials, *RandColor.ToString());
		}
		//UE_LOG(LogTemp, Warning, TEXT("\t\t\t%s::%d Add unique color %s"), TEXT(__FUNCTION__), __LINE__, *RandColor.ToString());
		return RandColor;
	}
}
//////////////////////////////////////////////////
//
//// Init
//void ASLVisLoggerSpectatorPC::MaskInit()
//{
//	UE_LOG(LogTemp, Warning, TEXT("%s::%d INIT"), TEXT(__FUNCTION__), __LINE__);
//	if (!bMaskInit && DefaultMaskMaterial)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("\t%s::%d IN INIT"), TEXT(__FUNCTION__), __LINE__);
//		auto AddDynamicMaskMaterialsLambda = [&](UMeshComponent* MC, const FString& HexColorVal)
//		{
//			TArray<UMaterialInterface*> Materials;
//			UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
//			for (int32 Idx = 0; Idx < MC->GetNumMaterials(); ++Idx)
//			{
//				DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
//					FLinearColor(FColor::FromHex(HexColorVal)));
//				Materials.Emplace(DynamicMaskMaterial);
//				UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Add MASK. Entity=%s; DynMatName=%s; Hex=%s;"),
//					TEXT(__FUNCTION__), __LINE__, *MC->GetOuter()->GetName(), *DynamicMaskMaterial->GetName(), *HexColorVal);
//			}
//			MeshesMaskMaterials.Emplace(MC, Materials);
//		};
//
//		// Iterate static meshes from the world
//		for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
//		{
//			if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
//			{
//				// Cache original materials
//				MeshesOrigMaterials.Add(SMC, SMC->GetMaterials());
//
//				// Check if the entity is annotated with a semantic color mask
//				FString ColorHex = FTags::GetValue(*SMAItr, "SemLog", "VisMask");
//				if (ColorHex.IsEmpty())
//				{
//					ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
//					if (ColorHex.IsEmpty())
//					{
//						UnknownMeshes.Emplace(SMC);
//					}
//					else
//					{
//						//AddDynamicMaskMaterialsLambda(SMC, ColorHex);
//
//						TArray<UMaterialInterface*> Materials;
//						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
//						for (int32 Idx = 0; Idx < SMC->GetNumMaterials(); ++Idx)
//						{
//							DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
//								FLinearColor(FColor::FromHex(ColorHex)));
//							Materials.Emplace(DynamicMaskMaterial);
//							UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Add MASK. Entity=%s; DynMatName=%s; Hex=%s;"),
//								TEXT(__FUNCTION__), __LINE__, *SMC->GetOuter()->GetName(), *DynamicMaskMaterial->GetName(), *ColorHex);
//						}
//						MeshesMaskMaterials.Emplace(SMC, Materials);
//					}
//				}
//				else
//				{
//					//AddDynamicMaskMaterialsLambda(SMC, ColorHex);
//
//					TArray<UMaterialInterface*> Materials;
//					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
//					for (int32 Idx = 0; Idx < SMC->GetNumMaterials(); ++Idx)
//					{
//						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
//							FLinearColor(FColor::FromHex(ColorHex)));
//						Materials.Emplace(DynamicMaskMaterial);
//						UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Add MASK. Entity=%s; DynMatName=%s; Hex=%s;"),
//							TEXT(__FUNCTION__), __LINE__, *SMC->GetOuter()->GetName(), *DynamicMaskMaterial->GetName(), *ColorHex);
//					}
//					MeshesMaskMaterials.Emplace(SMC, Materials);
//				}
//			}
//		}
//
//		// TODO include semantic data from skeletal meshes as well
//		// Iterate skeletal meshes
//		for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
//		{
//			if (USkeletalMeshComponent* SkMC = SkMAItr->GetSkeletalMeshComponent())
//			{
//				// Add original materials
//				MeshesOrigMaterials.Add(SkMC, SkMC->GetMaterials());
//
//				// Check if the entity is annotated with a semantic color mask
//				// TODO how to read bone colors
//				FString ColorHex = FTags::GetValue(*SkMAItr, "SemLog", "VisMask");
//				if (ColorHex.IsEmpty())
//				{
//					ColorHex = FTags::GetValue(SkMC, "SemLog", "VisMask");
//					if (ColorHex.IsEmpty())
//					{
//						UnknownMeshes.Emplace(SkMC);
//					}
//					else
//					{
//						//AddDynamicMaskMaterialsLambda(SkMC, ColorHex, InParent);
//					}
//				}
//				else
//				{
//					//AddDynamicMaskMaterialsLambda(SkMC, ColorHex, InParent);
//				}
//			}
//		}
//
//		for (auto& MeshMatPair : MeshesMaskMaterials)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d VIEW mask of %s"),
//				TEXT(__FUNCTION__), __LINE__, *MeshMatPair.Key->GetOuter()->GetName());
//			
//			for (int32 Idx = 0; Idx < MeshMatPair.Key->GetNumMaterials(); ++Idx)
//			{
//				UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
//					TEXT(__FUNCTION__), __LINE__, *Mat->GetName());
//				MeshMatPair.Key->SetMaterial(Idx, Mat);				
//			}
//		}
//
//
//		bMaskInit = true;
//	}
//}
//
//// Apply mask materials
//bool ASLVisLoggerSpectatorPC::ApplyMaskMaterials()
//{
//	if (!bIsInit)
//	{
//		return false;
//	}
//
//	UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Setting mask materials.."), TEXT(__FUNCTION__), __LINE__);
//	for (auto& MeshMatPair : MeshesMaskMaterials)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Apply mask to %s"),
//			TEXT(__FUNCTION__), __LINE__, *MeshMatPair.Key->GetOuter()->GetName());
//		for (int32 Idx = 0; Idx < MeshMatPair.Key->GetNumMaterials(); ++Idx)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
//				TEXT(__FUNCTION__), __LINE__, *Mat->GetName());
//			MeshMatPair.Key->SetMaterial(Idx, Mat);
//		}
//	}
//
//	for (auto& Mesh : UnknownMeshes)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Apply mask to %s"),
//			TEXT(__FUNCTION__), __LINE__, *Mesh->GetOuter()->GetName());
//		for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
//				TEXT(__FUNCTION__), __LINE__, *DefaultMaskMaterial->GetName());
//			Mesh->SetMaterial(Idx, DefaultMaskMaterial);
//		}
//	}
//	return true;
//}
//
//// Apply original materials
//bool ASLVisLoggerSpectatorPC::ApplyOriginalMaterials()
//{
//	if (!bIsInit)
//	{
//		return false;
//	}
//
//	UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Setting orig materials.."), TEXT(__FUNCTION__), __LINE__);
//	for (auto& MeshMatPair : MeshesMaskMaterials)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Apply mask to %s"),
//			TEXT(__FUNCTION__), __LINE__, *MeshMatPair.Key->GetOuter()->GetName());
//		int32 MatIdx = 0;
//		for (auto& Mat : MeshMatPair.Value)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
//				TEXT(__FUNCTION__), __LINE__, *Mat->GetName());
//			//MeshMatPair.Key->SetMaterial(MatIdx, Mat);
//			MatIdx++;
//		}
//	}
//	return true;
//}
