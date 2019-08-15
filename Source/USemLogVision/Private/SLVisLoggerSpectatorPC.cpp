// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLoggerSpectatorPC.h"
#include "Engine/DemoNetDriver.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "SLVisViewActor.h"
#include "SLVisMaskHandler.h"
#include "SLVisImageWriterMongoCxx.h"
#include "SLVisImageWriterMongoC.h"
#include "SLVisImageWriterFile.h"
#include "Animation/SkeletalMeshActor.h"
#include "Camera/CameraActor.h"
#include "Engine/LocalPlayer.h"

// Ctor
ASLVisLoggerSpectatorPC::ASLVisLoggerSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	CurrentViewIndex = 0;
	CurrRenderIndex = 0;
	DemoTimestamp = 0.f;

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Add buffer types to visualize
	RenderTypes.Add(ESLVisRenderType::Color);
	RenderTypes.Add(ESLVisRenderType::Depth);
	RenderTypes.Add(ESLVisRenderType::Mask);
	RenderTypes.Add(ESLVisRenderType::Normal);

	
	ScrubRate = 0.1f;
	SkipNewEntryDistance = 0.05f;


	// Image size 
	// 8k (7680, 4320) / 4k (3840, 2160) / 2k (2048, 1080) / fhd (1920, 1080) / hd (1280, 720) / sd (720, 480)
	Resolution.X = 500;
	Resolution.Y = 500;
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
	//ASLVisLoggerSpectatorPC::Finish();
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
		CurrentViewIndex = 0;
		CurrRenderIndex = 0;
		DemoTimestamp = 0.f;

		// Make sure the time offset is not larger than the replay update rate
		FMath::Clamp(SkipNewEntryDistance, 0.f, ScrubRate);

		if (RenderTypes.Contains(ESLVisRenderType::Mask))
		{
			MaskHandler = NewObject<USLVisMaskHandler>(this);
			MaskHandler->Init(Resolution.X * Resolution.Y);
			if (!MaskHandler->IsInit())
			{
				RenderTypes.Remove(ESLVisRenderType::Mask);
			}
		}

		// Continue only if driver and viewport is available
		NetDriver = GetWorld()->DemoNetDriver;
		ViewportClient = GetWorld()->GetGameViewport();
		if (NetDriver && ViewportClient)
		{
			// Set the writer pointer
			CreateWriter();

			// Flag as initialized
			if (Writer && Writer->IsInit())
			{
				// Add existing camera views to the array
				SetCameraViews();

				if (CameraViews.Num() > 0)
				{
					// Init the progress logger (num images saved etc.)
					ProgressLogger.Init(NetDriver->DemoTotalTime, ScrubRate, CameraViews.Num(), RenderTypes.Num());

					// Check which timestamp should be rendered first
					while (DemoTimestamp < NetDriver->DemoTotalTime && ShouldSkipThisFrame(DemoTimestamp))
					{
						// Update the number of saved images (even if timestamps are skipped, for tracking purposes)
						ProgressLogger.AddProcessedImaged(CameraViews.Num() * RenderTypes.Num());
						DemoTimestamp += ScrubRate;
						ProgressLogger.SetCurrentTime(DemoTimestamp);
					}

					// Set rendering parameters
					SetRenderingParameters();

					// Bind callback functions
					NetDriver->OnGotoTimeDelegate.AddUObject(this, &ASLVisLoggerSpectatorPC::ScrubCB);
					NetDriver->OnDemoFinishPlaybackDelegate.AddUObject(this, &ASLVisLoggerSpectatorPC::DemoFinishedCB);
					ViewportClient->OnScreenshotCaptured().AddUObject(this, &ASLVisLoggerSpectatorPC::ScreenshotCB);
					bIsInit = true;
				}
			}
		}
	}
}

// Start logger
void ASLVisLoggerSpectatorPC::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Disable physics on skeletal entities
		//DisablePhysicsOnEntities();

		// Start durations logger
		//DurationsLogger.SetStartTime();

		// Set camera to first target and first rendering type
		if (ASLVisLoggerSpectatorPC::GotoInitialViewTarget() && ASLVisLoggerSpectatorPC::GotoInitialRenderType())
		{
			// Go to the beginning of the demo and start requesting screenshots, unpause demo in order to scrub
			ASLVisLoggerSpectatorPC::DemoUnPause();
			NetDriver->GotoTimeInSeconds(DemoTimestamp);
			//DurationsLogger.SetScrubRequestTime();

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

		ProgressLogger.LogProgress();

		// Log the duration of the vision logger
		//DurationsLogger.SetEndTime(true);

		// Flag as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	
		// Crashes if called from EndPlay
		// Try to quit editor
		ASLVisLoggerSpectatorPC::QuitEditor();
	}
}

// Disable physics for skeletal components
void ASLVisLoggerSpectatorPC::DisablePhysicsOnEntities()
{
	for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d DISABLE PHYSICS ON %s"), *FString(__func__), __LINE__,*SMAItr->GetName());
		SMAItr->DisableComponentsSimulatePhysics();
	}

	for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d DISABLE PHYSICS ON %s"), *FString(__func__), __LINE__, *SkMAItr->GetName());
		SkMAItr->DisableComponentsSimulatePhysics();
	}
}

// Create data writer
void ASLVisLoggerSpectatorPC::CreateWriter()
{
	// Get episode id by removing suffix
	FString EpisodeId = NetDriver->GetActiveReplayName();
	EpisodeId.RemoveFromEnd("_RP");

	// Create writer
	//Writer = NewObject<USLVisImageWriterFile>(this);
	//Writer->Init(FSLVisImageWriterParams(FPaths::ProjectDir() + TEXT("/SemLog/Episodes/"), EpisodeId));

#if SLVIS_WITH_LIBMONGO_C
	Writer = NewObject<USLVisImageWriterMongoC>(this);
	Writer->Init(FSLVisImageWriterParams(FString(DBName), EpisodeId, SkipNewEntryDistance, "127.0.0.1", 27017));
#endif //SLVIS_WITH_LIBMONGO_C

#if SLVIS_WITH_LIBMONGO_CXX
	//Writer = NewObject<USLVisImageWriterMongoCxx>(this);
	//Writer->Init(FSLVisImageWriterParams(FString(DBName), EpisodeId, "127.0.0.1", 27017));
#endif //SLVIS_WITH_LIBMONGO_CXX


#if WITH_EDITOR
			//ProgressBar = MakeUnique<FScopedSlowTask>(NumImagesToSave, FText::FromString(FString(TEXT("Saving images.."))));
			//ProgressBar->MakeDialog(true, true);
#endif //WITH_EDITOR
}

// Cache existing camera views
void ASLVisLoggerSpectatorPC::SetCameraViews()
{
	// Get camera views from the world
	for (TActorIterator<ASLVisViewActor>Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->Init();
		if(Itr->IsInit())
		{
			CameraViews.Add(*Itr);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not initialized semantically"),
				*FString(__func__), __LINE__, *Itr->GetName());
		}
	}
}

// Set rendered image quality
void ASLVisLoggerSpectatorPC::SetRenderingParameters()
{	
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	// SetResolution() sets GIsHighResScreenshot to true, which triggers the callback, avoid this be re-setting the flat to false
	GIsHighResScreenshot = false;
	
	// TODO see if this is useful
	//GetHighResScreenshotConfig().ParseConsoleCommand();
	
	// TODO this does not seem to have an effect
	// Set the display resolution for the current game view. Has no effect in the editor
	// e.g. 1280x720w for windowed, 1920x1080f for fullscreen, 1920x1080wf for windowed fullscreen
	//FString ResStr = FString::FromInt(ResX) + "x" + FString::FromInt(ResY)/* + "f"*/;
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetRes"))->Set(*ResStr);

	IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFormat"))->Set(PF_R8G8B8A8);

	// Which anti-aliasing mode is used by default (if masking is used the AA should be turned off, otherwise the edge pixels will differ)
	// 	AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_FXAA);

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

// Request a screenshot
void ASLVisLoggerSpectatorPC::RequestScreenshot()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		GetHighResScreenshotConfig().FilenameOverride = FSLVisHelper::CreateImageFilename(DemoTimestamp,
			CameraViews[CurrentViewIndex]->GetClass(), RenderTypes[CurrRenderIndex]);
		//GetHighResScreenshotConfig().SetForce128BitRendering(true);
		//GetHighResScreenshotConfig().SetHDRCapture(true);
		ViewportClient->Viewport->TakeHighResScreenShot();
		//DurationsLogger.SetScreenshotRequestTime();
	});
}

// Called after a successful scrub
void ASLVisLoggerSpectatorPC::ScrubCB()
{
	// Log the duration of the scrub
	//DurationsLogger.SetScrubCbTime(true);

	// Pause the replay
	ASLVisLoggerSpectatorPC::DemoPause();

	// Request a screenshot
	ASLVisLoggerSpectatorPC::RequestScreenshot();
}

// Called when screenshot is captured
void ASLVisLoggerSpectatorPC::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
#if WITH_EDITOR
	//ProgressBar->EnterProgressFrame();
#endif //WITH_EDITOR

	// Log the duration of the screenshot processing
	//DurationsLogger.ScreenshotCbTime(true);

	// Make sure the current view data is init
	if (!CurrentViewData.IsInit())
	{
		CurrentViewData.Init(CameraViews[CurrentViewIndex]->GetId(), CameraViews[CurrentViewIndex]->GetClass(), Resolution);
	}

	// Remove const-ness from image
	TArray<FColor>& BitmapRef = const_cast<TArray<FColor>&>(Bitmap);

	// Get the start time for calculating the img data processing duration
	if (MaskHandler && MaskHandler->AreMasksOn())
	{
		// Process the semantic mask image, fix pixel color deviations in image, return entities data
		FTransform CurrViewWorldTransform = CameraViews[CurrentViewIndex]->GetTransform();
		MaskHandler->ProcessMaskImage(BitmapRef, CurrViewWorldTransform, CurrentViewData);
	}

	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, BitmapRef, CompressedBitmap);

	// Add current image data to the view
	CurrentViewData.ImagesData.Emplace(FSLVisImageData(RenderTypes[CurrRenderIndex], CompressedBitmap));

	// Check if multiple view types are required
	if (GotoNextRenderType())
	{
		RequestScreenshot();
	}
	else
	{
		// Go back to the first view type
		if (GotoInitialRenderType())
		{
			// Check if more views are available
			if (GotoNextViewTarget())
			{
				// Add and reset previous view data
				CurrentTsData.ViewsData.Emplace(CurrentViewData);
				CurrentViewData.Clear();

				// Request a new screenshot
				RequestScreenshot();
			}
			else
			{
				// Write data (no new images in this demo timestamp)
				CurrentTsData.ViewsData.Emplace(CurrentViewData);
				CurrentTsData.Timestamp = DemoTimestamp;
				Writer->Write(CurrentTsData);
				CurrentViewData.Clear();
				CurrentTsData.Reset();

				ProgressLogger.LogProgress();

				// Find the next timestamp to scrub to
				do {
					// Update the number of saved images (even if timestamps are skipped, for tracking purposes)
					ProgressLogger.AddProcessedImaged(CameraViews.Num() * RenderTypes.Num());
					DemoTimestamp += ScrubRate;
					ProgressLogger.SetCurrentTime(DemoTimestamp);
				} while (DemoTimestamp < NetDriver->DemoTotalTime &&
					ShouldSkipThisFrame(DemoTimestamp));

				// Go back to first view
				if (GotoInitialViewTarget())
				{
					// Unpause demo in order to scrub
					DemoUnPause();
					NetDriver->GotoTimeInSeconds(DemoTimestamp);
					//DurationsLogger.SetScrubRequestTime();
				}
			}
		}
	}
}

// Called when the replay is finished
void ASLVisLoggerSpectatorPC::DemoFinishedCB()
{
	ASLVisLoggerSpectatorPC::Finish();
}

// Pause demo (needed to take the screenshot)
void ASLVisLoggerSpectatorPC::DemoPause()
{
	// Else, it is already paused
	if (GetWorld()->GetWorldSettings()->Pauser == nullptr)
	{
		GetWorld()->GetWorldSettings()->Pauser = PlayerState;
	}
}

// Un-pause demo (needed to be able to move to another timestamp)
void ASLVisLoggerSpectatorPC::DemoUnPause()
{
	// Else, it is already running
	if (GetWorld()->GetWorldSettings()->Pauser != nullptr)
	{
		GetWorld()->GetWorldSettings()->Pauser = nullptr;
	}
}

// Sets the first view, returns false if there are no views at all
bool ASLVisLoggerSpectatorPC::GotoInitialViewTarget()
{
	CurrentViewIndex = 0;
	if (CameraViews.IsValidIndex(CurrentViewIndex))
	{
		SetViewTarget(CameraViews[CurrentViewIndex], FViewTargetTransitionParams());
		return true;
	}
	else
	{
		// 0 is not a valid index, i.e. array is empty
		return false;
	}
}

// Sets the next view, returns false there are no more views
bool ASLVisLoggerSpectatorPC::GotoNextViewTarget()
{
	CurrentViewIndex++;
	if (CameraViews.IsValidIndex(CurrentViewIndex))
	{
		SetViewTarget(CameraViews[CurrentViewIndex], FViewTargetTransitionParams());
		return true;
	}
	else
	{
		return false;
	}
}

// Sets the first visualization buffer type, false if none
bool ASLVisLoggerSpectatorPC::GotoInitialRenderType()
{
	CurrRenderIndex = 0;
	if (RenderTypes.IsValidIndex(CurrRenderIndex))
	{
		return ApplyRenderType(RenderTypes[CurrRenderIndex]);
	}
	else
	{
		// 0 is not a valid index, i.e. array is empty
		return false;
	}
}

// Sets the next visualization buffer type, returns false there are no more views
bool ASLVisLoggerSpectatorPC::GotoNextRenderType()
{
	CurrRenderIndex++;
	if (RenderTypes.IsValidIndex(CurrRenderIndex))
	{
		return ApplyRenderType(RenderTypes[CurrRenderIndex]);
	}
	else
	{
		return false;
	}
}

// Render the given view type
bool ASLVisLoggerSpectatorPC::ApplyRenderType(ESLVisRenderType RenderType)
{
	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));	

	// Choose rendering type
	if (BufferVisTargetCV && ViewportClient->GetEngineShowFlags())
	{
		if (RenderType == ESLVisRenderType::Color)
		{
			if (MaskHandler && MaskHandler->AreMasksOn())
			{
				MaskHandler->ApplyOriginalMaterials();
				ViewportClient->GetEngineShowFlags()->SetPostProcessing(true);
				//ViewportClient->GetEngineShowFlags()->SetLighting(true);
				//ViewportClient->GetEngineShowFlags()->SetColorGrading(true);
				//ViewportClient->GetEngineShowFlags()->SetTonemapper(true);
				//ViewportClient->GetEngineShowFlags()->SetMaterials(true);
				//ViewportClient->GetEngineShowFlags()->SetAntiAliasing(true);
			}
			// Visualize original scene
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else if (RenderType == ESLVisRenderType::Mask)
		{
			if (MaskHandler && MaskHandler->IsInit())
			{
				MaskHandler->ApplyMaskMaterials();
				ViewportClient->GetEngineShowFlags()->SetPostProcessing(false);
				//ViewportClient->GetEngineShowFlags()->SetLighting(false);
				//ViewportClient->GetEngineShowFlags()->SetColorGrading(false);
				//ViewportClient->GetEngineShowFlags()->SetTonemapper(false);
				//ViewportClient->GetEngineShowFlags()->SetMaterials(false);
				//ViewportClient->GetEngineShowFlags()->SetAntiAliasing(false);

				//ViewportClient->GetEngineShowFlags()->SetSceneColorFringe(false);
				//ViewportClient->GetEngineShowFlags()->SetAtmosphericFog(false);
				//ViewportClient->GetEngineShowFlags()->SetGlobalIllumination(true);

				////ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
				////BufferVisTargetCV->Set(TEXT("BaseColor"));
			}
		}
		else
		{
			if (MaskHandler && MaskHandler->AreMasksOn())
			{
				MaskHandler->ApplyOriginalMaterials();
				ViewportClient->GetEngineShowFlags()->SetPostProcessing(true);
				//ViewportClient->GetEngineShowFlags()->SetLighting(true);
				//ViewportClient->GetEngineShowFlags()->SetColorGrading(true);
				//ViewportClient->GetEngineShowFlags()->SetTonemapper(true);
				//ViewportClient->GetEngineShowFlags()->SetMaterials(true);
				//ViewportClient->GetEngineShowFlags()->SetAntiAliasing(true);
			}
			// Select buffer to visualize
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FSLVisHelper::GetRenderTypeAsCommandString(RenderType));
		}
		return true;
	}
	return false;
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
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Frame can be skipped.."), *FString(__func__), __LINE__);
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
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Frame can be skipped.."), *FString(__func__), __LINE__);
				return true;
			}
			else
			{
				return false;
			}
			//return AsMongoCWriter->ShouldSkipThisTimestamp(Timestamp);
		}
		// Don't skip anything
		return false;
	}

	// No valid writer, skip frames
	UE_LOG(LogTemp, Error, TEXT("%s::%d No valid writer found, skipping frames.."), *FString(__func__), __LINE__);
	return true;
}

// Shallow check if object is in frustum
void ASLVisLoggerSpectatorPC::ShallowFrustumCheck()
{
	if (IsInGameThread())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d In game thread, continuing.. "), *FString(__func__), __LINE__);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d RenderType=%s; TargetLabel:%s;\n World TimeSeconds=%f; DeltaTimeSeconds=%f;"),
			*FString(__func__), __LINE__, *FSLVisHelper::GetRenderTypeAsString(RenderTypes[CurrRenderIndex]),
			*CameraViews[CurrentViewIndex]->GetClass(),
			GetWorld()->TimeSeconds, GetWorld()->DeltaTimeSeconds);
	
		for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
		{
			if (ActItr->GetName().Contains("OvenKnob") || ActItr->GetName().Contains("FridgeArea"))
			{
				FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
					ViewportClient->Viewport,
					GetWorld()->Scene,
					ViewportClient->EngineShowFlags).SetRealtimeUpdate(true));

				if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
				{
					FVector ViewLocation;
					FRotator ViewRotation;
					FSceneView* SceneView = LocalPlayer->CalcSceneView(
						&ViewFamily, ViewLocation, ViewRotation, ViewportClient->Viewport);

					if (SceneView)
					{
						if (SceneView->ViewFrustum.IntersectBox(ActItr->GetActorLocation(), ActItr->GetComponentsBoundingBox().GetExtent()))
						{
							UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d %s Intersected"),
								*FString(__func__), __LINE__, *ActItr->GetName());

							// todo naive check via 5 traces from bounding box edges + center
							// if all traces are blocked, the object is not visible
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("\t\t%s::%d %s Not in view (not intersected)"),
								*FString(__func__), __LINE__, *ActItr->GetName());
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("\t%s::%d %s Not in view"),
							*FString(__func__), __LINE__, *ActItr->GetName());
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d No local player.."), *FString(__func__), __LINE__);
				}
			}
			
		}
	
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not in game thread, skipping.."), *FString(__func__), __LINE__);
	}
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d WORLD TimeSeconds=%f; DeltaTimeSeconds=%f"),
	//	*FString(__func__), __LINE__, GetWorld()->TimeSeconds, GetWorld()->DeltaTimeSeconds);
	//// Cache original materials, and create mask materials for static meshes
	//for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	//{
	//	if (ActItr->GetName().Contains("Oven") || ActItr->GetName().Contains("Fridge"))
	//	{
	//		if (ActItr->WasRecentlyRendered(0.01))
	//		{
	//			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s was recently rendered, last render time: %f"),
	//				*FString(__func__), __LINE__, *ActItr->GetName(), ActItr->GetLastRenderTime());
	//		}
	//		else 
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d %s was NOT recently rendered, last render time: %f"),
	//				*FString(__func__), __LINE__, *ActItr->GetName(), ActItr->GetLastRenderTime());
	//		}
	//	}
	//}




	

	UE_LOG(LogTemp, Error, TEXT("%s::%d  ** * * ** * * ** * * ** * * *"), *FString(__func__), __LINE__);
}
