// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLoggerSpectatorPC.h"
#include "Engine/DemoNetDriver.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "SLVisCameraView.h"
#include "SLVisImageWriterMongoCxx.h"
#include "SLVisImageWriterMongoC.h"
#include "SLVisImageWriterFile.h"

// Ctor
ASLVisLoggerSpectatorPC::ASLVisLoggerSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	DemoUpdateRate = 0.18f;
	NewEntryTimeRange = 0.12f; 
	ActiveCameraViewIndex = 0;
	ActiveViewTypeIndex = 0;
	DemoTimestamp = 0.f;
	NumImagesSaved = 0;
	NumImagesToSave = 0;

	// Add buffer types to visualize
	ViewTypes.Add(NAME_None); // Default will be color
	ViewTypes.Add("SceneDepth");
	ViewTypes.Add("WorldNormal");

	// Image size
	ResX = 12;
	ResY = 8;
}

// Called when the game starts or when spawned
void ASLVisLoggerSpectatorPC::BeginPlay()
{
	Super::BeginPlay();

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
			Writer = NewObject<USLVisImageWriterMongoC>(this);
			Writer->Init(FSLVisImageWriterParams(
				TEXT("SemLog"), EpisodeId, NewEntryTimeRange, "127.0.0.1", 27017));

			//Writer = NewObject<USLVisImageWriterMongoCxx>(this);
			//Writer->Init(FSLVisImageWriterParams(
			//	TEXT("SemLog"), EpisodeId, "127.0.0.1", 27017));

			//Writer = NewObject<USLVisImageWriterFile>(this);
			//Writer->Init(FSLVisImageWriterParams(
			//	FPaths::ProjectDir() + TEXT("/SemLog/Episodes/"), EpisodeId));
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
				NumImagesToSave = NumTimesteps * NumImages;

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
		}

		// Flag as started
		bIsStarted = true;
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

		UE_LOG(LogTemp, Warning, TEXT("%s::%d Duration: %f/%f; Imgs: %d/%d;"),
			TEXT(__FUNCTION__), __LINE__,
			DemoTimestamp, NetDriver->DemoTotalTime,
			NumImagesSaved, NumImagesToSave);

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
}

// Called after a successful scrub
void ASLVisLoggerSpectatorPC::DemoGotoCB()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Duration: %f/%f; Imgs: %d/%d;"),
		TEXT(__FUNCTION__), __LINE__,
		DemoTimestamp, NetDriver->DemoTotalTime,
		NumImagesSaved, NumImagesToSave);

	// Pause the replay
	ASLVisLoggerSpectatorPC::DemoPause();

	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
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
	ImagesAtTimestamp.Emplace(FSLVisImageData(FSLVisImageMetadata(ViewTypes[ActiveViewTypeIndex],
				CameraViews[ActiveCameraViewIndex]->GetCameraLabel(),ResX, ResY), CompressedBitmap));

#if WITH_EDITOR
	//ProgressBar->EnterProgressFrame();
#endif //WITH_EDITOR

	// Check if multiple view types are required
	if (ASLVisLoggerSpectatorPC::SetNextViewType())
	{
		// Request screenshot on game thread
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
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
					ViewportClient->Viewport->TakeHighResScreenShot();
				});
			}
			else
			{
				// We reached the last camera + view in this time slice
				// Write data
				Writer->Write(DemoTimestamp, ImagesAtTimestamp);

				// Update the number of saved images
				NumImagesSaved += ImagesAtTimestamp.Num();

				// Clear previous data array
				ImagesAtTimestamp.Empty();

				// Check if images should be rendered for the given timestamp (false if not a mongo writer)
				do 
				{
					DemoTimestamp += DemoUpdateRate;
				} while (ASLVisLoggerSpectatorPC::ShouldSkipThisTimestamp(DemoTimestamp));

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
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(ViewType == NAME_None ? false : true);
			ViewportClient->GetEngineShowFlags()->SetTonemapper(ViewType == NAME_None ? false : true);
			ICVarVisTarget->Set(*ViewType.ToString());
			return true;
		}
	}
	return false;
}

// Skip the current timestamp (return false if not a mongo writer)
bool ASLVisLoggerSpectatorPC::ShouldSkipThisTimestamp(float Timestamp)
{
	if (Writer)
	{
		if (USLVisImageWriterMongoCxx* AsMongoWriter = Cast<USLVisImageWriterMongoCxx>(Writer.GetObject()))
		{
			return AsMongoWriter->ShouldSkipThisTimestamp(Timestamp);
		}
		else if (USLVisImageWriterMongoC* AsMongoCWriter = Cast<USLVisImageWriterMongoC>(Writer.GetObject()))
		{
			return AsMongoCWriter->ShouldSkipThisTimestamp(Timestamp);
		}
	}
	return false;
}