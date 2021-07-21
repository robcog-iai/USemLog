// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Meta/SLMetaScanner.h"
#include "Meta/SLMetaScannerStructs.h"
#include "Monitors/SLContactMonitorInterface.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "FileHelper.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Light.h"
#include "Engine/Engine.h"
#include "Engine/DirectionalLight.h"


// Ctor
USLMetaScanner::USLMetaScanner()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bWithItemRelativeCameraDistance = false;
	CurrViewModeIdx = INDEX_NONE;
	CurrPoseIdx = INDEX_NONE;
	CurrItemIdx = INDEX_NONE;
	TempItemPixelNum = INDEX_NONE;
	PrevViewMode = ESLMetaScannerViewMode::NONE;

	ViewModes.Add(ESLMetaScannerViewMode::Color);
	ViewModes.Add(ESLMetaScannerViewMode::Unlit);
	ViewModes.Add(ESLMetaScannerViewMode::White);
	ViewModes.Add(ESLMetaScannerViewMode::Depth);
	ViewModes.Add(ESLMetaScannerViewMode::Normal);
}

// Dtor
USLMetaScanner::~USLMetaScanner()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();
	}
}

// Init scanner
void USLMetaScanner::Init(const FString& InTaskId, FSLMetaScannerParams ScanParams)
{
	if (!bIsInit)
	{
		//// Check if owner is of type USLMetadataLogger, used to access the mongodb calls
		//if(USLMetadataLogger* MLOuter = Cast<USLMetadataLogger>(GetOuter()))
		//{
		//	MetadataLoggerParent = MLOuter;
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Parent is not of type USLMetadataLogger.. aborting.."), *FString(__func__), __LINE__);
		//}
		
		// Check if the scans should be stored locally as well
		if(ScanParams.bIncludeScansLocally)
		{
			SaveLocallyFolderName = "/SemLog/" + InTaskId + "/3dscan/";
		}

		// If no view modes are available, add a default one
		if(ViewModes.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No view modes found, added default one (lit).."), *FString(__func__), __LINE__);
			ViewModes.Add(ESLMetaScannerViewMode::Color);
		}

		if (!LoadScanPoses(ScanParams.NumberOfScanPoints, ScanParams.CameraDistanceToScanItem))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No scan poses loaded.."), *FString(__func__), __LINE__);
			return;
		}

		if (!LoadScanItems(ScanParams.MaxItemVolume, ScanParams.CameraDistanceToScanItem, bScanItemsOnlyWithSLContact))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No scan items loaded.."), *FString(__func__), __LINE__);
			return;
		}

		// Spawn helper actors, items and scan poses, skip items larger that the distance to camera
		if (!SpawnCameraPoseActorDummy())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load a camera dummy mesh for moving the target view.."), *FString(__func__), __LINE__);
			return;
		}

		// Spawn extra lights to avoid dark corneres
		SpawnExtraLights();

		// Setup actor clones with custom material (white)
		if (ViewModes.Contains(ESLMetaScannerViewMode::White))
		{
			if(!LoadMaskMaterial() || !CreateWhiteMaterialClones())
			{
				ViewModes.Remove(ESLMetaScannerViewMode::White);
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup clones with white material.."), *FString(__func__), __LINE__);
			}
		}

		// Used for the screenshot requests
		ViewportClient = GetWorld()->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}

		// Move the first item in position
		if(!SetupFirstScanItem())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set the first item for scanning.."), *FString(__func__), __LINE__);
			return;
		}

		// TODO different approach, store gridfs oids in an array, and update the db after each finished item
		// Open the first scan document in mongodb
		//MetadataLoggerParent->StartScanEntry(ScanItems[CurrItemIdx].Value, ScanParams.Resolution.X, ScanParams.Resolution.Y);

		// Set the screenshot resolution;
		InitScreenshotResolution(ScanParams.Resolution);
		
		// Set rendering parameters
		InitRenderParameters();

		// Bind the screenshot callback
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLMetaScanner::ScreenshotCB);

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLMetaScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
		// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr
		if(!GotoFirstScanPose())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not trigger the first scan.."), *FString(__func__), __LINE__);
			return;
		}

		// Store the first scan camera pose into the data
		ScanPoseData.CameraPose = CameraPoseActor->GetActorTransform(); //ScanPoses[CurrPoseIdx];

		// Cannot be called in Init() because GetFirstPlayerController() is nullptr before BeginPlay
		SetupFirstViewMode();

		// Hide default pawn
		GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);

		// Start the dominoes
		RequestScreenshot();

		bIsStarted = true;
	}
}

// Finish scanning
void USLMetaScanner::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Request a screenshot
void USLMetaScanner::RequestScreenshot()
{
	CurrScanName = /*FString::FromInt(CurrItemIdx) + "_" + */ScanItems[CurrItemIdx].Value + "_" + FString::FromInt(CurrPoseIdx);
	if (!ViewModeString.IsEmpty())
	{
		CurrScanName.Append("_" + ViewModeString);
	}

	GetHighResScreenshotConfig().FilenameOverride = CurrScanName;
	//GetHighResScreenshotConfig().SetForce128BitRendering(true);
	//GetHighResScreenshotConfig().SetHDRCapture(true);

	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when screenshot is captured
void USLMetaScanner::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	PrintProgress();

	// Count and check how many pixels does the item occupy in the image (works with view mode mask/unlit)
	//CountItemPixelNumWithCheck(Bitmap);

	// Add the number of pixels that the item occupies to the doc
	if (ViewModes[CurrViewModeIdx] == ESLMetaScannerViewMode::White)
	{
		ScanToolkit.GetItemPixelNumAndBB(Bitmap, SizeX, SizeY, ScanPoseData.NumPixels, ScanPoseData.MinBB, ScanPoseData.MaxBB);
		//GetItemPixelNumAndBB(Bitmap, SizeX, SizeY, ScanPoseData.NumPixels, ScanPoseData.MinBB, ScanPoseData.MaxBB);
	}

	//// Remove const-ness from array
	//TArray<FColor>& BitmapRef = const_cast<TArray<FColor>&>(Bitmap)

	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);

	// Add image current scan data
	ScanPoseData.Images.Emplace(GetViewModeName(ViewModes[CurrViewModeIdx]), CompressedBitmap);

	// Save the png locally
	if (!SaveLocallyFolderName.IsEmpty())
	{
		SaveImageLocally(CompressedBitmap);
	}

	// Item and camera in position, check for other view modes
	if (SetupNextViewMode())
	{
		RequestScreenshot();
	}
	else
	{
		// No other view modes, keep or set the first one
		SetupFirstViewMode();

		// New camera or image location will be set, reset the number of item pixels
		TempItemPixelNum = INDEX_NONE;

		// Check for next camera poses
		if (GotoNextScanPose())
		{
			//MetadataLoggerParent->AddScanPoseEntry(ScanPoseData);
			ScanPoseData.Images.Empty();
			ScanPoseData.CameraPose = CameraPoseActor->GetActorTransform(); //ScanPoses[CurrPoseIdx];

			RequestScreenshot();
		}
		else
		{
			//MetadataLoggerParent->AddScanPoseEntry(ScanPoseData);
			ScanPoseData.Images.Empty();
			//MetadataLoggerParent->FinishScanEntry();

			if (SetupNextItem())
			{
				// If in mask mode, apply the mask material on the current item as well
				if (ViewModes[CurrViewModeIdx] == ESLMetaScannerViewMode::White)
				{
					OriginalMaterials.Empty();
					ApplyMaskMaterial();
				}

				// No other scan poses found, set next item and first camera scan pose
				GotoFirstScanPose();

				//MetadataLoggerParent->StartScanEntry(ScanItems[CurrItemIdx].Value, SizeX, SizeY);
				ScanPoseData.CameraPose = CameraPoseActor->GetActorTransform(); //ScanPoses[CurrPoseIdx];

				RequestScreenshot();
			}
			else
			{
				// No more items, or camera scan poses
				UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Finished scanning.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
				QuitEditor();
			}
		}
	}
}

// Set view mode
bool USLMetaScanner::SetupFirstViewMode()
{
	CurrViewModeIdx = 0;
	if (!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		return false;
	}

	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Setup next view mode
bool USLMetaScanner::SetupNextViewMode()
{
	CurrViewModeIdx++;
	if (!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		return false;
	}

	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Setup first item in the scan box
bool USLMetaScanner::SetupFirstScanItem()
{
	CurrItemIdx = 0;
	if (!ScanItems.IsValidIndex(CurrItemIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid item index [%ld].."), *FString(__func__), __LINE__, CurrItemIdx);
		return false;
	}

	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(false);

	if (bWithItemRelativeCameraDistance)
	{
		CurrItemCameraDistance = GetItemRelativeCameraDistance(ScanItems[CurrItemIdx].Key);
	}
	return true;
}

// Set next item in the scan box, return false if there are no more items
bool USLMetaScanner::SetupNextItem()
{
	// Hide previous actor
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(true);

	// Bring next item
	CurrItemIdx++;
	if (!ScanItems.IsValidIndex(CurrItemIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last item [%ld] reached.."), *FString(__func__), __LINE__, CurrItemIdx-1);
		return false;
	}

	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(false);

	if (bWithItemRelativeCameraDistance)
	{
		CurrItemCameraDistance = GetItemRelativeCameraDistance(ScanItems[CurrItemIdx].Key);
	}
	return true;
}

// Scan item from the first camera pose (cannot be called before BeginPlay, GetFirstPlayerController() is nullptr)
bool USLMetaScanner::GotoFirstScanPose()
{
	CurrPoseIdx = 0;
	if (!ScanPoses.IsValidIndex(CurrPoseIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid pose index [%ld].."), *FString(__func__), __LINE__, CurrPoseIdx);
		return false;
	}

	if (bWithItemRelativeCameraDistance)
	{
		FTransform TempT = ScanPoses[CurrPoseIdx];
		TempT.SetTranslation(TempT.GetTranslation() * CurrItemCameraDistance);
		CameraPoseActor->SetActorTransform(TempT);
	}
	else
	{
		CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
	}

	GetWorld()->GetFirstPlayerController()->SetViewTarget(CameraPoseActor); // Cannot be called before BeginPlay
	return true;
}

// Scan item from the next camera pose, return false if there are no more poses
bool USLMetaScanner::GotoNextScanPose()
{
	CurrPoseIdx++;
	if (!ScanPoses.IsValidIndex(CurrPoseIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last pose [%ld] reached .."), *FString(__func__), __LINE__, CurrPoseIdx-1)
		return false;
	}

	if (bWithItemRelativeCameraDistance)
	{
		FTransform TempT = ScanPoses[CurrPoseIdx];
		TempT.SetTranslation(TempT.GetTranslation() * CurrItemCameraDistance);
		CameraPoseActor->SetActorTransform(TempT);
	}
	else
	{
		CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
	}

	GetWorld()->GetFirstPlayerController()->SetViewTarget(CameraPoseActor);
	return true;
}

// Load scan camera convenience actor
bool USLMetaScanner::SpawnCameraPoseActorDummy()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanCameraPoseDummy");
	CameraPoseActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnParams);
	if(!CameraPoseActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn convenience camera pose actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	CameraPoseActor->SetActorLabel(FString(TEXT("SM_ScanCameraPoseDummy")));
#endif // WITH_EDITOR

	UStaticMesh* CameraPoseDummyMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/USemLog/CV/ScanCameraPoseDummy/SM_ScanCameraPoseDummy.SM_ScanCameraPoseDummy"));
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

// Spawn extra lights to avoid dark corners on images
void USLMetaScanner::SpawnExtraLights()
{
	FActorSpawnParameters SpawnParams;
	FTransform SpawnTransform;

	/* Top light */
	SpawnParams.Name = TEXT("SL_ExtraLight_Top");
	SpawnTransform = FTransform(FRotator(-90.f, 0.f, 0.f), FVector(0.f, 0.f, 300.f));
	ADirectionalLight* TopLight = GetWorld()->SpawnActor<ADirectionalLight>(
		ADirectionalLight::StaticClass(), SpawnTransform, SpawnParams);
	if (TopLight)
	{
		TopLight->SetMobility(EComponentMobility::Movable);
		TopLight->GetLightComponent()->SetIntensity(1.6f);
#if WITH_EDITOR
		TopLight->SetActorLabel(FString(TEXT("SL_ExtraLight_Top")));
#endif // WITH_EDITOR
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn extra light.."), *FString(__func__), __LINE__);
	}

	/* Bottom light */
	SpawnParams.Name = TEXT("SL_ExtraLight_Bottom");
	SpawnTransform = FTransform(FRotator(90.f, 0.f, 0.f), FVector(0.f, 0.f, -300.f));
	ADirectionalLight* BottomLight = GetWorld()->SpawnActor<ADirectionalLight>(
		ADirectionalLight::StaticClass(), SpawnTransform, SpawnParams);
	if (BottomLight)
	{
		BottomLight->SetMobility(EComponentMobility::Movable);
		BottomLight->GetLightComponent()->SetIntensity(2.2f);
#if WITH_EDITOR
		BottomLight->SetActorLabel(FString(TEXT("SL_ExtraLight_Bottom")));
#endif // WITH_EDITOR
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn extra light.."), *FString(__func__), __LINE__);
	}

}

// Load scanning points
bool USLMetaScanner::LoadScanPoses(int32 NumScanPose, float DistanceToCamera)
{
	if(DistanceToCamera < 0.01f)
	{
		bWithItemRelativeCameraDistance = true;
		ScanToolkit.GenerateSphereScanPoses(NumScanPose, 1, ScanPoses);
	}
	else
	{
		ScanToolkit.GenerateSphereScanPoses(NumScanPose, DistanceToCamera, ScanPoses);
	}
	
	if(ScanPoses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No scan poses added.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Load items to scan
bool USLMetaScanner::LoadScanItems(float MaxVolume, float MaxBoundsLength, bool bWithContactMonitor)
{
	// Cache the classes that were already iterated
	TSet<FString> ConsultedClasses;

	//// Iterate all actors
	//for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	//{
	//	// Check if the item has a visual
	//	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(*ActItr))
	//	{
	//		// Make all actors movable without physics, detached from any other components/actors
	//		// -- avoids errors of moving attached, or actors with attachments
	//		AsSMA->DisableComponentsSimulatePhysics();
	//		AsSMA->SetMobility(EComponentMobility::Movable);
	//		AsSMA->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//		//AsSMA->DetachAllSceneComponents(AsSMA->GetRootComponent(), FDetachmentTransformRules::KeepWorldTransform);
	//		TArray<AActor*> AttachedActors;
	//		AsSMA->GetAttachedActors(AttachedActors);
	//		for(auto& A : AttachedActors)
	//		{
	//			A->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//			//A->DetachAllSceneComponents(A->GetRootComponent(), FDetachmentTransformRules::KeepWorldTransform);
	//		}
	//		
	//		// Everything is hidden by default
	//		AsSMA->SetActorHiddenInGame(true);
	//		
	//		// Skip if class was already checked (or empty)
	//		const FString Class = FTags::GetValue(*ActItr, "SemLog", "Class");
	//		if(Class.IsEmpty() || ConsultedClasses.Contains(Class))
	//		{
	//			continue;
	//		}
	//		ConsultedClasses.Emplace(Class);

	//		// Make sure the static mesh component is valid
	//		if(UStaticMeshComponent* SMC = AsSMA->GetStaticMeshComponent())
	//		{
	//			if(HasScanningRequirements(SMC, MaxVolume, MaxBoundsLength, bWithContactMonitor, false))
	//			{
	//				//SMC->SetSimulatePhysics(false);
	//				//AsSMA->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//				//AsSMA->DetachAllSceneComponents(AsSMA->GetRootComponent(), FDetachmentTransformRules::KeepWorldTransform);
	//				//SMC->SetMobility(EComponentMobility::Movable);
	//				ActItr->SetActorTransform(FTransform::Identity);
	//				
	//				ScanItems.Emplace(SMC, Class);
	//				UE_LOG(LogTemp, Warning, TEXT("%s::%d Added %s to the scan items list.."),
	//					*FString(__func__), __LINE__, *Class);
	//			}
	//			else
	//			{
	//				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %s does not meet scanning requirements, skipping item scan.."),
	//					*FString(__func__), __LINE__, *Class);
	//			}
	//		}
	//		else
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no static mesh component, skipping item scan.."),
	//				*FString(__func__), __LINE__, *Class);
	//		}
	//	}
	//	else if(!Cast<ALight>(*ActItr))
	//	{
	//		// Hide everything else that is not a light
	//		(*ActItr)->SetActorHiddenInGame(true);
	//	}
	//}

	if(ScanItems.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No items found to scan.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

// Load mask dynamic material
bool USLMetaScanner::LoadMaskMaterial()
{
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this,
		TEXT("/USemLog/CV/M_SLDefaultMask.M_SLDefaultMask"));
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."),
			*FString(__func__), __LINE__);
		return false;
	}
	
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Create the dynamic mask material from the default one
	DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
	DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::White);
	return true;
}

// Create clones of the items with mask material on top
bool USLMetaScanner::CreateWhiteMaterialClones()
{
	if(ScanItems.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find any items to clone.."), *FString(__func__), __LINE__);
		return false;
	}

	int32 EmplaceIdx = 0;
	for(const auto& Pair : ScanItems)
	{
		if(AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Pair.Key->GetOwner()))
		{
			FActorSpawnParameters Parameters;
			Parameters.Template = SMA;
			Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			//Parameters.Instigator = SMA->GetInstigator();
			Parameters.Name = FName(*(SMA->GetName() + TEXT("_MaskClone")));
			AStaticMeshActor* SMAClone =  GetWorld()->SpawnActor<AStaticMeshActor>(SMA->GetClass(), Parameters);

			if(UStaticMeshComponent* SMC = SMAClone->GetStaticMeshComponent())
			{
				for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
				{
					SMC->SetMaterial(MatIdx, DynamicMaskMaterial);
				}
			}
			SMAClone->SetActorHiddenInGame(true);
			MaskClones.EmplaceAt(EmplaceIdx, SMAClone);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
		EmplaceIdx++;
	}
	if(MaskClones.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No mask clones where generated.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Init hi-res screenshot resolution
void USLMetaScanner::InitScreenshotResolution(FIntPoint Resolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;
}

// Init render parameters (resolution, view mode)
void USLMetaScanner::InitRenderParameters()
{
	//// Set screenshot image and viewport resolution size
	//GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	//// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	//GIsHighResScreenshot = false;

	// Defines the memory layout used for the GBuffer,
	// 0: lower precision (8bit per component, for profiling), 1: low precision (default)
	// 3: high precision normals encoding, 5: high precision
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.GBufferFormat"))->Set(5);

	
	// Set the near clipping plane (in cm)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetNearClipPlane"))->Set(0); // Not a console variable, but a command
	//GNearClippingPlane = 0; // View is distorted after finishing the scanning
	if(GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("r.SetNearClipPlane 0"));
	}
	
	// AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_None);

	// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// LOD level to force, -1 is off. (0 - Best)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Apply view mode
void USLMetaScanner::ApplyViewMode(ESLMetaScannerViewMode Mode)
{
	// No change in the rendering view mode
	if(Mode == PrevViewMode)
	{
		return;
	}

	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if(Mode == ESLMetaScannerViewMode::Color)
	{
		if(PrevViewMode == ESLMetaScannerViewMode::Depth || PrevViewMode == ESLMetaScannerViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::White)
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		ViewModeString = "C";
	}
	else if(Mode == ESLMetaScannerViewMode::Unlit)
	{
		if(PrevViewMode == ESLMetaScannerViewMode::Color)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::White)
		{
			ApplyOriginalMaterial();
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Depth || PrevViewMode == ESLMetaScannerViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		ViewModeString = "U";
	}
	else if(Mode == ESLMetaScannerViewMode::White)
	{
		if(PrevViewMode == ESLMetaScannerViewMode::Unlit)
		{
			ApplyMaskMaterial();
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Color)
		{
			ApplyMaskMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Depth || PrevViewMode == ESLMetaScannerViewMode::Normal)
		{
			ApplyMaskMaterial();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ApplyMaskMaterial();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		ViewModeString = "M";
	}
	else if(Mode == ESLMetaScannerViewMode::Depth)
	{
		if(PrevViewMode == ESLMetaScannerViewMode::White)
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Normal)
		{
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		ViewModeString = "D";
	}
	else if(Mode == ESLMetaScannerViewMode::Normal)
	{
		if(PrevViewMode == ESLMetaScannerViewMode::Depth)
		{
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::White)
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLMetaScannerViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		ViewModeString = "N";
	}

	// Cache as previous view mode
	PrevViewMode = Mode;
}

// Apply mask material to current item
void USLMetaScanner::ApplyMaskMaterial()
{
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(true);
	MaskClones[CurrItemIdx]->SetActorHiddenInGame(false);
	
	//if(OriginalMaterials.Num() == 0)
	//{
	//	UStaticMeshComponent* SMC = ScanItems[CurrItemIdx].Key;
	//	OriginalMaterials = SMC->GetMaterials();
	//	
	//	//FColor MaskColor(FColor::FromHex(ScanItems[CurrItemIdx].Value.VisualMask));
	//	//FLinearColor LinMaskColorSRGB(FLinearColor::FromSRGBColor(MaskColor));
	//	//FLinearColor LinMaskColorPow22(FLinearColor::FromPow22Color(MaskColor));
	//	//UE_LOG(LogTemp, Warning, TEXT("%s::%d InHex=%s; OutColor=%s; OutHex=%s;"),
	//	//	*FString(__func__), __LINE__, *ScanItems[CurrItemIdx].Value.VisualMask, *MaskColor.ToString(), *MaskColor.ToHex());
	//	//UE_LOG(LogTemp, Warning, TEXT("%s::%d LinMaskColorSRGB=%s; LinMaskColorPow22=%s; "),
	//	//	*FString(__func__), __LINE__, *LinMaskColorSRGB.ToString(), *LinMaskColorPow22.ToString());
	//	//DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromPow22Color(MaskColor));
	//	////DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
	//	
	//	for (int32 Idx = 0; Idx < SMC->GetNumMaterials(); ++Idx)
	//	{
	//		SMC->SetMaterial(Idx, DynamicMaskMaterial);
	//	}
	//}
}

// Apply original material to current item
void USLMetaScanner::ApplyOriginalMaterial()
{
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(false);
	MaskClones[CurrItemIdx]->SetActorHiddenInGame(true);

	//if(OriginalMaterials.Num() > 0)
	//{
	//	int32 Idx = 0;
	//	for(const auto& M : OriginalMaterials)
	//	{
	//		ScanItems[CurrItemIdx].Key->SetMaterial(Idx, M);
	//	}
	//	OriginalMaterials.Empty();
	//}
}

// Quit the editor once the scanning is finished
void USLMetaScanner::QuitEditor()
{
	//FGenericPlatformMisc::RequestExit(false);
	//
	//FGameDelegates::Get().GetExitCommandDelegate().Broadcast();
	//FPlatformMisc::RequestExit(0);

#if WITH_EDITOR	
	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
	}
#endif // WITH_EDITOR
}

// Save the compressed screenshot image locally
void USLMetaScanner::SaveImageLocally(const TArray<uint8>& CompressedBitmap)
{
	FString ItemClassFolder = ScanItems[CurrItemIdx].Value + "_" + ViewModeString + "/";
	FString Path = FPaths::ProjectDir() + SaveLocallyFolderName + ItemClassFolder + CurrScanName + ".png";
	FPaths::RemoveDuplicateSlashes(Path);
	FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
}

// Output progress to terminal
void USLMetaScanner::PrintProgress() const
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d [%f] Progress=[Item=%ld/%ld; Pose=%ld/%ld; ViewMode=%ld/%ld; Scan=%ld/%ld]"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
		CurrItemIdx + 1, ScanItems.Num(),
		CurrPoseIdx + 1, ScanPoses.Num(),
		CurrViewModeIdx + 1, ViewModes.Num(),
		CurrItemIdx * ScanPoses.Num() * ViewModes.Num() + CurrPoseIdx * ViewModes.Num() + CurrViewModeIdx + 1,
		ScanItems.Num() * ScanPoses.Num() * ViewModes.Num());
}

// Get the view mode string name
FString USLMetaScanner::GetViewModeName(ESLMetaScannerViewMode Mode) const
{
	if(Mode == ESLMetaScannerViewMode::Color)
	{
		return FString("Color");
	}
	else if(Mode == ESLMetaScannerViewMode::Unlit)
	{
		return FString("Unlit");
	}
	else if(Mode == ESLMetaScannerViewMode::White)
	{
		return FString("Mask");
	}
	else if(Mode == ESLMetaScannerViewMode::Depth)
	{
		return FString("Depth");
	}
	else if(Mode == ESLMetaScannerViewMode::Normal)
	{
		return FString("Normal");
	}
	else
	{
		return FString("None");
	}
}

// Check if the items meets the requirements to be scanned
float USLMetaScanner::GetItemRelativeCameraDistance(UStaticMeshComponent* SMC) const
{
	return SMC->Bounds.SphereRadius * 1.5f;
}

// Check against varios properties if the item should be scanned
bool USLMetaScanner::HasScanningRequirements(UStaticMeshComponent* SMC, float MaxVolume, float MaxBoundsLength,
	bool bWithSemanticContactMonitor, bool bOnlyMovable) const
{
	bool bShouldBeScanned = true;
	if(bOnlyMovable && SMC->Mobility != EComponentMobility::Movable)
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t %s is not movable, skipping item scan.."),
		//	*FString(__func__), __LINE__, *SMC->GetOwner()->GetName());
		return false;
	}

	if(bWithSemanticContactMonitor && !HasSemanticContactMonitor(SMC))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t %s has no semantic contact shape, skipping item scan.."),
		//	*FString(__func__), __LINE__, *SMC->GetOwner()->GetName());
		return false;
	}

	// If MaxVolume is 0, all items should be considered
	if(bShouldBeScanned && MaxVolume > 0.01f)
	{
		bShouldBeScanned = SMC->Bounds.GetBox().GetVolume() < MaxVolume;
	}

	// If bounds length is 0, consider all items
	if(bShouldBeScanned && MaxBoundsLength > 0.01f)
	{
		bShouldBeScanned = SMC->Bounds.SphereRadius < MaxBoundsLength;
	}

	return bShouldBeScanned;
}

// Check if the item is wrapped in a semantic contact shape (has a SLContactMonitorInterface sibling)
bool USLMetaScanner::HasSemanticContactMonitor(UStaticMeshComponent* SMC) const
{
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
	TArray<UActorComponent*> Components;
	SMC->GetOwner()->GetComponents(UShapeComponent::StaticClass(), Components);
	for (const auto C : Components)
	{
		if (Cast<ISLContactMonitorInterface>(C))
		{
			return true;
		}
	}
#else
	for(const auto C : SMC->GetOwner()->GetComponentsByClass(UShapeComponent::StaticClass()))
	{
		if (Cast<ISLContactMonitorInterface>(C))
		{
			return true;
		}
	}
#endif
	return false;
}

