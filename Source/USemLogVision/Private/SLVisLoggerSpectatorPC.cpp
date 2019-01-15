// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLoggerSpectatorPC.h"
#include "SLVisCamera.h"
#include "Engine/DemoNetDriver.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"
#include "HighResScreenshot.h"
//#include "UnrealClient.h"
//#include "GameDelegates.h"
//#include "TimerManager.h"
//#include "GenericPlatform/GenericPlatformMisc.h"
#include "SLVisImageWriterMongo.h"
#include "SLVisImageWriterFile.h"

// Ctor
ASLVisLoggerSpectatorPC::ASLVisLoggerSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	UpdateRate = 0.2214f;
	ViewTargetIndex = -1;
	ViewTypeIndex = -1;
	DemoTimestamp = 0.f;
	NumberOfSavedImages = 0;
	NumberOfTotalImages = 0;
	FilenameSuffix = TEXT("");
}

// Called when the game starts or when spawned
void ASLVisLoggerSpectatorPC::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld()->DemoNetDriver && GetWorld()->DemoNetDriver->IsPlaying())
	{
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
	ASLVisLoggerSpectatorPC::Finish();
}

// Called to bind functionality to input
void ASLVisLoggerSpectatorPC::SetupInputComponent()
{
	Super::SetupInputComponent();
	//InputComponent->BindAction("FinishPlayback", IE_Pressed, this, &ASLVisLoggerSpectatorPC::Finish).bExecuteWhenPaused = true;
}

// Called every frame
 void ASLVisLoggerSpectatorPC::Tick(float DeltaTime)
 {
	 Super::Tick(DeltaTime);

	 if (bIsInit)
	 {
		 UE_LOG(LogTemp, Warning, TEXT("%s::%d Demo: %f/%f; Imgs: %d/%d; World: Play=%f; Real=%f"),
			 TEXT(__FUNCTION__), __LINE__,
			 DemoTimestamp, NetDriver->DemoTotalTime,
			 NumberOfSavedImages, NumberOfTotalImages,
			 GetWorld()->GetTimeSeconds(),
			 GetWorld()->GetRealTimeSeconds());
	 }
 }

// Init logger
void ASLVisLoggerSpectatorPC::Init()
{
	if (!bIsInit)
	{
		// Continue only if driver and viewport is available
		NetDriver = GetWorld()->DemoNetDriver;
		ViewportClient = GetWorld()->GetGameViewport();
		if (NetDriver && ViewportClient)
		{
			// Create writer
#if SLVIS_WITH_LIBMONGO
			Writer = NewObject<USLVisImageWriterMongo>(this);
			Writer->Init(FSLVisImageWriterParams(
				"SemLog", GetWorld()->DemoNetDriver->GetActiveReplayName(), "127.0.0.1", 27017));
#else
			Writer = NewObject<USLVisImageWriterFile>(this);
			Writer->Init(FSLVisImageWriterParams(
				"SemLog", GetWorld()->DemoNetDriver->GetActiveReplayName()));
#endif //SLVIS_WITH_LIBMONGO

			// Set rendering parameters
			ASLVisLoggerSpectatorPC::SetupRenderingProperties();

			// Set the array of the camera positions
			for (TActorIterator<ASLVisCamera>Itr(GetWorld()); Itr; ++Itr)
			{
				Cameras.Add(*Itr);
			}

			// Add buffer types to visualize
			ViewTypes.Add(NAME_None); // Default will be color
			ViewTypes.Add("SceneDepth");
			ViewTypes.Add("WorldNormal");

			// Set path to store the images
			EpisodePath = FPaths::ProjectDir() + TEXT("/SemLog/Episodes/") + GetWorld()->DemoNetDriver->GetActiveReplayName() + TEXT("/");
			FPaths::RemoveDuplicateSlashes(EpisodePath);

			NumberOfTotalImages = (uint32)(NetDriver->DemoTotalTime / UpdateRate) * Cameras.Num() * ViewTypes.Num();

			// Bind callback functions
			NetDriver->OnGotoTimeDelegate.AddUObject(this, &ASLVisLoggerSpectatorPC::DemoGotoCB);
			NetDriver->OnDemoFinishPlaybackDelegate.AddUObject(this, &ASLVisLoggerSpectatorPC::Finish);
			ViewportClient->OnScreenshotCaptured().AddUObject(this, &ASLVisLoggerSpectatorPC::ScreenshotCB);

			// Flag as initialized
			bIsInit = true;
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
		//FGenericPlatformMisc::RequestExit(false);
		//
		//FGameDelegates::Get().GetExitCommandDelegate().Broadcast();
		//FPlatformMisc::RequestExit(0);

		if (GEngine)
		{
			//GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
		}

		// Flag as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Set rendered image quality
void ASLVisLoggerSpectatorPC::SetupRenderingProperties()
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(640, 480, 1.0f);

	// Set the display resolution for the current game view. Has no effect in the editor
	// e.g. 1280x720w for windowed, 1920x1080f for fullscreen, 1920x1080wf for windowed fullscreen
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetRes"))->Set(TEXT("640x480"));

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

	// Write data
	Writer->Write(CompressedBitmap, DemoTimestamp, ViewTypes[ViewTypeIndex], ViewTargetIndex);

	// Set path and filename
	FString Ts = FString::SanitizeFloat(DemoTimestamp).Replace(TEXT("."), TEXT("-"));
	FString CurrFilename = FString::Printf(TEXT("V%d_%s_%s.png"), ViewTargetIndex, *Ts, *FilenameSuffix);
	FString CurrFullPath = EpisodePath + "/" + CurrFilename;
	FPaths::RemoveDuplicateSlashes(CurrFullPath);

	// Save to file
	FFileHelper::SaveArrayToFile(CompressedBitmap, *CurrFullPath);
	NumberOfSavedImages++;

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
				// Go back to the first camera target and advance in the demo
				DemoTimestamp += UpdateRate;
				if (ASLVisLoggerSpectatorPC::SetFirstViewTarget())
				{
					// Unpause demo to be able to scrub
					ASLVisLoggerSpectatorPC::DemoUnPause();
					NetDriver->GotoTimeInSeconds(DemoTimestamp);
				}
			}
		}
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
	ViewTargetIndex = 0;
	if (Cameras.IsValidIndex(ViewTargetIndex))
	{
		SetViewTarget(Cameras[ViewTargetIndex], FViewTargetTransitionParams());
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
	ViewTargetIndex++;
	if (Cameras.IsValidIndex(ViewTargetIndex))
	{
		SetViewTarget(Cameras[ViewTargetIndex], FViewTargetTransitionParams());
		return true;
	}
	else
	{
		ViewTargetIndex = -1;
		return false;
	}
}

// Sets the first visualization buffer type, false if none
bool ASLVisLoggerSpectatorPC::SetFirstViewType()
{
	ViewTypeIndex = 0;
	if (ViewTypes.IsValidIndex(ViewTypeIndex))
	{
		return ASLVisLoggerSpectatorPC::SetupViewType(ViewTypes[ViewTypeIndex]);
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
	ViewTypeIndex++;
	if (ViewTypes.IsValidIndex(ViewTypeIndex))
	{
		return ASLVisLoggerSpectatorPC::SetupViewType(ViewTypes[ViewTypeIndex]);
	}
	else
	{
		ViewTypeIndex = -1;
		return false;
	}
}

// Render the given view type
bool ASLVisLoggerSpectatorPC::SetupViewType(FName ViewType)
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
			return ASLVisLoggerSpectatorPC::SetupFilenameSuffix(ViewType);
		}
	}
	return false;
}

// Set the suffix of the file depending on the view type
bool ASLVisLoggerSpectatorPC::SetupFilenameSuffix(FName ViewType)
{
	if (ViewType.IsEqual(NAME_None))
	{
		FilenameSuffix = "C"; // Color
		return true;
	}
	else if (ViewType.IsEqual("SceneDepth"))
	{
		FilenameSuffix = "D"; // Depth
		return true;
	}
	else if (ViewType.IsEqual("WorldNormal"))
	{
		FilenameSuffix = "N"; // Normal
		return true;
	}
	else
	{
		// Unsupported buffer type
		return false;
	}
}