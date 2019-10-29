// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLItemScanner.h"
#include "SLEntitiesManager.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "FileHelper.h"
#include "Components/StaticMeshComponent.h"

// Ctor
USLItemScanner::USLItemScanner()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bUnlit = false;
	bIncludeLocally = false;
	bIsTickable = false;
	CurrPoseIdx = INDEX_NONE;
	CurrItemIdx = INDEX_NONE;
}

// Dtor
USLItemScanner::~USLItemScanner()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();
	}
}

// Init scanner
void USLItemScanner::Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort,
		 UWorld* InWorld, FIntPoint Resolution, bool bScanViewModeUnlit, bool bIncludeScansLocally, bool bOverwrite)
{
	if (!bIsInit)
	{
		Location = InTaskId;
		bUnlit = bScanViewModeUnlit;
		bIncludeLocally = bIncludeScansLocally;
		
		// Cache the world, needed later to get the camera manager (can be done after init -- around BeginPlay)
		World = InWorld;

		ViewportClient = World->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}

		// Bind the screenshot result callback
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLItemScanner::ScreenshotCB);

		// Spawn the scan box mesh actor
		if(!LoadScanBoxActor())
		{
			return;
		}

		// Spawn the target view dummy actor
		if(!LoadScanCameraPoseActor())
		{
			return;
		}

		// Load the scan points pattern
		if(!LoadScanPoints())
		{
			return;
		}
		
		// Load items which need scanning
		if(!LoadItemsToScan())
		{
			return;
		}

		// Set scan image resolution and various rendering paramaters
		InitRenderParameters(Resolution);

		// Disable physics on skeletal actors (avoid any of them roaming through the scene)
		for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
		{
			SkMAItr->DisableComponentsSimulatePhysics();
		}
		
		bIsInit = true;
	}
}

// Start scanning
void USLItemScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
		// Cannot be called in Init() because GetFirstPlayerController() is nullptr before BeginPlay
		SetupViewMode();
		
		if(!SetupFirstScanItem())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set the first item for scanning.."), *FString(__func__), __LINE__);
			return;
		}

		if(!SetupFirstScanPose())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not trigger the first scan.."), *FString(__func__), __LINE__);
			return;
		}

		RequestScreenshot();
		
		// Enable ticking (camera movement will happen every tick)
		bIsTickable = true;
		
		bIsStarted = true;
	}
}

// Finish scanning
void USLItemScanner::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Finish"),
			*FString(__func__), __LINE__);
		
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLItemScanner::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Item/Pose Idx=[%d/%d]"),
			*FString(__func__), __LINE__, World->GetTimeSeconds(),
			CurrPoseIdx, CurrItemIdx);
	
	//if(CurrItemIdx < ScanItems.Num())
	//{
	//	
	//}
	//
	//if(CurrPoseIdx < ScanPoses.Num())
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] CurrCameraPoseIdx = %ld; CameraPose=%s; ActorPose=%s;"),
	//		*FString(__func__), __LINE__, World->GetTimeSeconds(),
	//		CurrPoseIdx,
	//		*ScanPoses[CurrPoseIdx].ToString(),
	//		*CameraPoseActor->GetTransform().ToString());
	//	
	//	CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
	//	PCM->SetViewTarget(CameraPoseActor);

	//	CurrPoseIdx++;
	//}
	//else
	//{
	//	QuitEditor();
	//}
}

// Return if object is ready to be ticked
bool USLItemScanner::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLItemScanner::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLWorldStateLogger, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */

// Load scan box actor
bool USLItemScanner::LoadScanBoxActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanBox");
	ScanBoxActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		FTransform(FVector(0.f, 0.f, ScanBoxOffsetZ)), SpawnParams);
	if(!ScanBoxActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn scan box actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	ScanBoxActor->SetActorLabel(FString(TEXT("SM_ScanBox")));
#endif // WITH_EDITOR
		
	UStaticMesh* ScanBoxMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/USemLog/Vision/ScanBox/SM_ScanBox.SM_ScanBox"));
	if(!ScanBoxMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find scan box mesh.."), *FString(__func__), __LINE__);
		ScanBoxActor->Destroy();
		return false;
	}
	ScanBoxActor->GetStaticMeshComponent()->SetStaticMesh(ScanBoxMesh);
	ScanBoxActor->SetMobility(EComponentMobility::Static);
	//ScanBoxActor->SetActorHiddenInGame(true);
	return true;
}

// Load scan camera convenience actor
bool USLItemScanner::LoadScanCameraPoseActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanCameraPoseDummy");
	CameraPoseActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnParams);
	if(!CameraPoseActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn convenience camera pose actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	CameraPoseActor->SetActorLabel(FString(TEXT("SM_ScanCameraPoseDummy")));
#endif // WITH_EDITOR

	UStaticMesh* CameraPoseDummyMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/USemLog/Vision/ScanCameraPoseDummy/SM_ScanCameraPoseDummy.SM_ScanCameraPoseDummy"));
	if(!CameraPoseDummyMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find camera pose dummy mesh.."), *FString(__func__), __LINE__);
		CameraPoseActor->Destroy();
		return false;
	}
	CameraPoseActor->GetStaticMeshComponent()->SetStaticMesh(CameraPoseDummyMesh);
	CameraPoseActor->SetMobility(EComponentMobility::Movable);

	return true;
}

// Load scanning points
bool USLItemScanner::LoadScanPoints()
{
	//// Load the scan poses
	//for(uint32 Idx = 0; Idx < 1200; Idx++)
	//{
	//	const float Z = Idx * 1.1f;
	//	const FTransform T = FTransform(FVector(0.f, 0.f, Idx));
	//	ScanPoses.Emplace(T);
	//}

	ScanPoses.Emplace(FTransform(FVector(-100.f, 0.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-100.f, 50.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-100.f, -50.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-50.f, 0.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-50.f, 50.f, ScanBoxOffsetZ)));
	ScanPoses.Emplace(FTransform(FVector(-50.f, -50.f, ScanBoxOffsetZ)));
		
	if(ScanPoses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No scan poses added.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Load items to scan
bool USLItemScanner::LoadItemsToScan()
{
	// Cache the classes that were already iterated
	TSet<FString> IteratedClasses;
	
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
	{
		const FSLEntity SemEntity = Pair.Value;

		// Skip if class was already checked
		if(IteratedClasses.Contains(SemEntity.Class))
		{
			continue;
		}
		IteratedClasses.Emplace(SemEntity.Class);
		
		// Check if the item has a visual
		if (AStaticMeshActor* ObjAsSMA = Cast<AStaticMeshActor>(SemEntity.Obj))
		{
			if(UStaticMeshComponent* SMC = ObjAsSMA->GetStaticMeshComponent())
			{
				if(ShouldBeScanned(SMC))
				{
					SMC->SetSimulatePhysics(false);
					ScanItems.Emplace(SMC->GetOwner(), SemEntity);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s is out of bounds/static, skipping scan.."),
						*FString(__func__), __LINE__, *SemEntity.Class);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no static mesh component, skipping scan.."),
					*FString(__func__), __LINE__, *SemEntity.Class);
			}
		}
		else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(SemEntity.Obj))
		{
			if(ShouldBeScanned(ObjAsSMC))
			{
				ObjAsSMC->SetSimulatePhysics(false);
				ScanItems.Emplace(ObjAsSMC->GetOwner(), SemEntity);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is out of bounds, skipping scan.."),
					*FString(__func__), __LINE__, *SemEntity.Class);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual, skipping scan.."),
				*FString(__func__), __LINE__, *SemEntity.Class);
		}
	}

	if(ScanItems.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No items found to scan.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

// Init render parameters (resolution, view mode)
void USLItemScanner::InitRenderParameters(FIntPoint Resolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;

	// 	AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_None);

	// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// LOD level to force, -1 is off. (0 - Best)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Set view mode
void USLItemScanner::SetupViewMode()
{
	if(bUnlit)
	{
		World->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
	}
	else
	{
		World->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
	}
}

// Setup first item in the scan box
bool USLItemScanner::SetupFirstScanItem()
{
	// Move the first item to the center of the scan box
	CurrItemIdx = 0;
	if(!ScanItems.IsValidIndex(CurrItemIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid item index [%ld].."), *FString(__func__), __LINE__, CurrItemIdx);
		return false;
	}
	ScanItems[CurrItemIdx].Key->SetActorLocation(ScanBoxActor->GetActorLocation());
	return true;
}

// Set next item in the scan box, return false if there are no more items
bool USLItemScanner::SetupNextItem()
{
	// Move previous item away
	ScanItems[CurrItemIdx].Key->SetActorLocation(FVector(0.f));
	
	// Bring next item
	CurrItemIdx++;
	if(!ScanItems.IsValidIndex(CurrItemIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last item [%ld] reached.."), *FString(__func__), __LINE__, CurrItemIdx-1);
		return false;
	}
	ScanItems[CurrItemIdx].Key->SetActorLocation(ScanBoxActor->GetActorLocation());
	return true;
}

// Scan item from the first camera pose
bool USLItemScanner::SetupFirstScanPose()
{
	CurrPoseIdx = 0;
	if(!ScanPoses.IsValidIndex(CurrPoseIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid pose index [%ld].."), *FString(__func__), __LINE__, CurrPoseIdx);
		return false;
	}
	CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
	World->GetFirstPlayerController()->SetViewTarget(CameraPoseActor); // Cannot be called before BeginPlay
	return true;
}

// Scan item from the next camera pose, return false if there are no more poses
bool USLItemScanner::SetupNextScanPose()
{
	CurrPoseIdx++;
	if(!ScanPoses.IsValidIndex(CurrPoseIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last pose [%ld] reached .."), *FString(__func__), __LINE__, CurrPoseIdx-1);
		return false;
	}
	CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
	World->GetFirstPlayerController()->SetViewTarget(CameraPoseActor);
	return true;
}

// Request a screenshot
void USLItemScanner::RequestScreenshot()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		CurrScanName = FString::FromInt(CurrItemIdx) + "_" + ScanItems[CurrItemIdx].Value.Class + "_" + FString::FromInt(CurrPoseIdx);
		GetHighResScreenshotConfig().FilenameOverride = CurrScanName;
		//GetHighResScreenshotConfig().SetForce128BitRendering(true);
		//GetHighResScreenshotConfig().SetHDRCapture(true);
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when screenshot is captured
void USLItemScanner::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Progress=[Items=%ld/%ld; Poses=%ld/%ld; Total=%ld/%ld]"),
		*FString(__func__), __LINE__, World->GetTimeSeconds(),
		CurrItemIdx + 1, ScanItems.Num(),
		CurrPoseIdx + 1, ScanPoses.Num(),
		CurrItemIdx * ScanPoses.Num() + CurrPoseIdx + 1,
		ScanItems.Num() * ScanPoses.Num());

	// Remove const-ness from image
	TArray<FColor>& BitmapRef = const_cast<TArray<FColor>&>(Bitmap);

	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, BitmapRef, CompressedBitmap);
	
	// Save the png locally
	if(bIncludeLocally)
	{
		FString Path = FPaths::ProjectDir() + "/SemLog/" + Location + "/Meta/" + CurrScanName + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	// Goto next step
	if(SetupNextScanPose())
	{
		RequestScreenshot();
	}
	else if(SetupNextItem())
	{
		SetupFirstScanPose();
		RequestScreenshot();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Finished scanning.."),
			*FString(__func__), __LINE__, World->GetTimeSeconds());
		QuitEditor();
	}
}

// Check if the items meets the requirements to be scanned
bool USLItemScanner::ShouldBeScanned(UStaticMeshComponent* SMC) const
{
	return SMC->Mobility == EComponentMobility::Movable &&
		SMC->Bounds.GetBox().GetVolume() < VolumeLimit &&
		SMC->Bounds.SphereRadius * 2.f < LengthLimit;
}

// Quit the editor once the scanning is finished
void USLItemScanner::QuitEditor()
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