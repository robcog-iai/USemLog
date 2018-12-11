// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLoggerSpectatorPC.h"
#include "SLVisCamera.h"
#include "Engine/DemoNetDriver.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"
//#include "UnrealClient.h"
//#include "GameDelegates.h"
//#include "TimerManager.h"
//#include "GenericPlatform/GenericPlatformMisc.h"


// Ctor
ASLVisLoggerSpectatorPC::ASLVisLoggerSpectatorPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	UpdateRate = 0.5f;
	ViewIndex = -1;
	DemoTimeSeconds = 0.f;
	NumberOfSavedImages = -1;
	NumberOfTotalImages = -1;
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
			 DemoTimeSeconds, NetDriver->DemoTotalTime,
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
			// Set the array of the camera positions
			for (TActorIterator<ASLVisCamera>Itr(GetWorld()); Itr; ++Itr)
			{
				Cameras.Add(*Itr);
			}

			// Set path to store the images
			EpisodePath = FPaths::ProjectDir() + TEXT("/SemLog/Episodes/") + GetWorld()->DemoNetDriver->GetActiveReplayName() + TEXT("/");
			FPaths::RemoveDuplicateSlashes(EpisodePath);

			NumberOfTotalImages = (uint32)(NetDriver->DemoTotalTime / UpdateRate) * Cameras.Num();

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
		// Go to the start of the replay and the first view target
		if (ASLVisLoggerSpectatorPC::SetFirstViewTarget())
		{
			NetDriver->GotoTimeInSeconds(DemoTimeSeconds);
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

// Called after a successful scrub
void ASLVisLoggerSpectatorPC::DemoGotoCB()
{	
	// Pause the replay
	ASLVisLoggerSpectatorPC::DemoPause();

	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread,[this]()
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

	// Set path and filename
	FString Ts = FString::SanitizeFloat(DemoTimeSeconds).Replace(TEXT("."), TEXT("-"));
	FString CurrFilename = FString::Printf(TEXT("V%d_%s.png"), ViewIndex, *Ts);
	FString CurrFullPath = EpisodePath + "/" + CurrFilename;
	FPaths::RemoveDuplicateSlashes(CurrFullPath);

	// Save to file
	FFileHelper::SaveArrayToFile(CompressedBitmap, *CurrFullPath);
	NumberOfSavedImages++;

	// Check if more views are available
	if (SetNextViewTarget())
	{
		// Take screenshot
		AsyncTask(ENamedThreads::GameThread,[this]() 
		{
			ViewportClient->Viewport->TakeHighResScreenShot();
		});
	}
	else
	{
		// Go to the first view target and the next timestamp
		DemoTimeSeconds += UpdateRate;		
		if (ASLVisLoggerSpectatorPC::SetFirstViewTarget())
		{
			// Unpause demo to be able to scrub
			ASLVisLoggerSpectatorPC::DemoUnPause();
			NetDriver->GotoTimeInSeconds(DemoTimeSeconds);
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
	ViewIndex = 0;
	if (Cameras.IsValidIndex(ViewIndex))
	{
		SetViewTarget(Cameras[ViewIndex]);
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
	ViewIndex++;
	if (Cameras.IsValidIndex(ViewIndex))
	{
		SetViewTarget(Cameras[ViewIndex]);
		return true;
	}
	else
	{
		ViewIndex = -1;
		return false;
	}
}
