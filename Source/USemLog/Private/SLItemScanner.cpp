// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLItemScanner.h"
#include "Tags.h"
#include "SLContactShapeInterface.h"
#include "SLMetadataLogger.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "FileHelper.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Light.h"
#include "Engine/Engine.h"

// Ctor
USLItemScanner::USLItemScanner()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bIncludeLocally = false;
	bWithItemRelativeCameraDistance = false;
	CurrViewModeIdx = INDEX_NONE;
	CurrPoseIdx = INDEX_NONE;
	CurrItemIdx = INDEX_NONE;
	TempItemPixelNum = INDEX_NONE;
	PrevViewMode = ESLItemScannerViewMode::NONE;
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
void USLItemScanner::Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort, FSLItemScanParams ScanParams)
{
	if (!bIsInit)
	{
		// Check if owner is of type USLMetadataLogger, used to access the mongodb calls
		if(USLMetadataLogger* MLOuter = Cast<USLMetadataLogger>(GetOuter()))
		{
			MetadataLoggerParent = MLOuter;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Parent is not of type USLMetadataLogger.. aborting.."), *FString(__func__), __LINE__);
		}
		
		FolderName = InTaskId;
		ViewModes = ScanParams.ViewModes;
		bIncludeLocally = ScanParams.bIncludeScansLocally;

		// If no view modes are available, add a default one
		if(ViewModes.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No view modes found, added default one (lit).."), *FString(__func__), __LINE__);
			ViewModes.Add(ESLItemScannerViewMode::Color);
		}

		// Spawn helper actors, items and scan poses, skip items larger that the distance to camera
		if(!LoadScanCameraPoseActor() 
			|| !LoadScanPoses(ScanParams.NumberOfScanPoints, ScanParams.CameraDistanceToScanItem)
			|| !LoadScanItems(ScanParams.MaxItemVolume, ScanParams.CameraDistanceToScanItem, bScanItemsOnlyWithSLContact))
		{
			return;
		}

		if (ViewModes.Contains(ESLItemScannerViewMode::Mask))
		{
			if(LoadMaskMaterial())
			{
				SetupMaskClones();
			}
			else
			{
				ViewModes.Remove(ESLItemScannerViewMode::Mask);
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

		// Open the first scan document in mongodb
		MetadataLoggerParent->StartScanEntry(ScanItems[CurrItemIdx].Value, ScanParams.Resolution.X, ScanParams.Resolution.Y);

		// Set the screenshot resolution;
		InitScreenshotResolution(ScanParams.Resolution);
		
		// Set rendering parameters
		InitRenderParameters();

		// Bind the screenshot callback
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLItemScanner::ScreenshotCB);

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLItemScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
		// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr
		if(!SetupFirstScanPose())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not trigger the first scan.."), *FString(__func__), __LINE__);
			return;
		}

		// Store the first scan camera pose into the data
		ScanPoseData.Pose = ScanPoses[CurrPoseIdx];

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
void USLItemScanner::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d Finish"), *FString(__func__), __LINE__);
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Load scan camera convenience actor
bool USLItemScanner::LoadScanCameraPoseActor()
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
bool USLItemScanner::LoadScanPoses(int32 NumScanPose, float DistanceToCamera)
{
	if(DistanceToCamera < 0.01f)
	{
		bWithItemRelativeCameraDistance = true;
		GenerateSphereScanPoses(NumScanPose, 1, ScanPoses);
	}
	else
	{
		GenerateSphereScanPoses(NumScanPose, DistanceToCamera, ScanPoses);
	}
	
	if(ScanPoses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No scan poses added.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Load items to scan
bool USLItemScanner::LoadScanItems(float MaxVolume, float MaxBoundsLength, bool bWithContactShape)
{
	// Cache the classes that were already iterated
	TSet<FString> ConsultedClasses;

	// Iterate all actors
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Check if the item has a visual
		if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(*ActItr))
		{
			// Make all actors movable without physics, detached from any other components/actors
			// -- avoids errors of moving attached, or actors with attachments
			AsSMA->DisableComponentsSimulatePhysics();
			AsSMA->SetMobility(EComponentMobility::Movable);
			AsSMA->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			//AsSMA->DetachAllSceneComponents(AsSMA->GetRootComponent(), FDetachmentTransformRules::KeepWorldTransform);
			TArray<AActor*> AttachedActors;
			AsSMA->GetAttachedActors(AttachedActors);
			for(auto& A : AttachedActors)
			{
				A->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				//A->DetachAllSceneComponents(A->GetRootComponent(), FDetachmentTransformRules::KeepWorldTransform);
			}
			
			// Everything is hidden by default
			AsSMA->SetActorHiddenInGame(true);
			
			const FString Class = FTags::GetValue(*ActItr, "SemLog", "Class");

			// Skip if class was already checked
			if(ConsultedClasses.Contains(Class))
			{
				continue;
			}
			ConsultedClasses.Emplace(Class);

			// Make sure the static mesh component is valid
			if(UStaticMeshComponent* SMC = AsSMA->GetStaticMeshComponent())
			{
				if(HasScanningRequirements(SMC, MaxVolume, MaxBoundsLength, bWithContactShape, false))
				{
					//SMC->SetSimulatePhysics(false);
					//AsSMA->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
					//AsSMA->DetachAllSceneComponents(AsSMA->GetRootComponent(), FDetachmentTransformRules::KeepWorldTransform);
					//SMC->SetMobility(EComponentMobility::Movable);
					ActItr->SetActorTransform(FTransform::Identity);
					
					ScanItems.Emplace(SMC, Class);
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Added %s to the scan items list.."),
						*FString(__func__), __LINE__, *Class);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d \t %s does not meet scanning requirements, skipping item scan.."),
						*FString(__func__), __LINE__, *Class);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no static mesh component, skipping item scan.."),
					*FString(__func__), __LINE__, *Class);
			}
		}
		else if(!Cast<ALight>(*ActItr))
		{
			// Hide everything else that is not a light
			(*ActItr)->SetActorHiddenInGame(true);
		}
	}

	if(ScanItems.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No items found to scan.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

// Load mask dynamic material
bool USLItemScanner::LoadMaskMaterial()
{
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this,
		TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
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
bool USLItemScanner::SetupMaskClones()
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
	return true;
}

// Init hi-res screenshot resolution
void USLItemScanner::InitScreenshotResolution(FIntPoint Resolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;
}

// Init render parameters (resolution, view mode)
void USLItemScanner::InitRenderParameters()
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

// Set view mode
bool USLItemScanner::SetupFirstViewMode()
{
	CurrViewModeIdx = 0;
	if(!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		return false;
	}
	
	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Setup next view mode
bool USLItemScanner::SetupNextViewMode()
{
	CurrViewModeIdx++;
	if(!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		return false;
	}

	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Setup first item in the scan box
bool USLItemScanner::SetupFirstScanItem()
{
	CurrItemIdx = 0;
	if(!ScanItems.IsValidIndex(CurrItemIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid item index [%ld].."), *FString(__func__), __LINE__, CurrItemIdx);
		return false;
	}
	
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(false);
	
	if(bWithItemRelativeCameraDistance)
	{
		CurrItemCameraDistance = GetItemRelativeCameraDistance(ScanItems[CurrItemIdx].Key);
	}
	return true;
}

// Set next item in the scan box, return false if there are no more items
bool USLItemScanner::SetupNextItem()
{
	// Hide previous actor
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(true);
	
	// Bring next item
	CurrItemIdx++;
	if(!ScanItems.IsValidIndex(CurrItemIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last item [%ld] reached.."), *FString(__func__), __LINE__, CurrItemIdx-1);
		return false;
	}

	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorHiddenInGame(false);
	
	if(bWithItemRelativeCameraDistance)
	{
		CurrItemCameraDistance = GetItemRelativeCameraDistance(ScanItems[CurrItemIdx].Key);
	}
	return true;
}

// Scan item from the first camera pose (cannot be called before BeginPlay, GetFirstPlayerController() is nullptr)
bool USLItemScanner::SetupFirstScanPose()
{
	CurrPoseIdx = 0;
	if(!ScanPoses.IsValidIndex(CurrPoseIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid pose index [%ld].."), *FString(__func__), __LINE__, CurrPoseIdx);
		return false;
	}
	
	if(bWithItemRelativeCameraDistance)
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
bool USLItemScanner::SetupNextScanPose()
{
	CurrPoseIdx++;
	if(!ScanPoses.IsValidIndex(CurrPoseIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last pose [%ld] reached .."), *FString(__func__), __LINE__, CurrPoseIdx-1)
		return false;
	}

	if(bWithItemRelativeCameraDistance)
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

// Request a screenshot
void USLItemScanner::RequestScreenshot()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		CurrScanName = FString::FromInt(CurrItemIdx) + "_" + ScanItems[CurrItemIdx].Value + "_" + FString::FromInt(CurrPoseIdx);
		if(!ViewModePostfix.IsEmpty())
		{
			CurrScanName.Append("_" + ViewModePostfix);
		}

		GetHighResScreenshotConfig().FilenameOverride = CurrScanName;
		//GetHighResScreenshotConfig().SetForce128BitRendering(true);
		//GetHighResScreenshotConfig().SetHDRCapture(true);
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when screenshot is captured
void USLItemScanner::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d [%f] Progress=[Item=%ld/%ld; Pose=%ld/%ld; ViewMode=%ld/%ld; Scan=%ld/%ld]"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
		CurrItemIdx + 1, ScanItems.Num(),
		CurrPoseIdx + 1, ScanPoses.Num(),
		CurrViewModeIdx + 1, ViewModes.Num(),
		CurrItemIdx * ScanPoses.Num() * ViewModes.Num() + CurrPoseIdx * ViewModes.Num() + CurrViewModeIdx + 1,
		ScanItems.Num() * ScanPoses.Num() * ViewModes.Num());

	// Count and check how many pixels does the item occupy in the image (works with view mode mask/unlit)
	//CountItemPixelNumWithCheck(Bitmap);

	// Add the number of pixels that the item occupies to the doc
	if(ViewModes[CurrViewModeIdx] == ESLItemScannerViewMode::Mask)
	{
		GetItemPixelNumAndBB(Bitmap, SizeX, SizeY, ScanPoseData.NumPixels, ScanPoseData.BBMin, ScanPoseData.BBMax);
	}
	
	//// Remove const-ness from array
	//TArray<FColor>& BitmapRef = const_cast<TArray<FColor>&>(Bitmap)
	
	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);

	// Add image to gridfs
	ScanPoseData.Images.Emplace(GetViewModeName(ViewModes[CurrViewModeIdx]), CompressedBitmap);

	// Save the png locally
	if(bIncludeLocally)
	{
		FString Path = FPaths::ProjectDir() + "/SemLog/" + FolderName + "/Meta/" + CurrScanName + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	// Item and camera in position, check for other view modes
	if(SetupNextViewMode())
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
		if(SetupNextScanPose())
		{
			MetadataLoggerParent->AddScanPoseEntry(ScanPoseData);
			ScanPoseData.Images.Empty();
			ScanPoseData.Pose = ScanPoses[CurrPoseIdx];
			
			RequestScreenshot();
		}
		else
		{
			MetadataLoggerParent->AddScanPoseEntry(ScanPoseData);
			ScanPoseData.Images.Empty();
			MetadataLoggerParent->FinishScanEntry();
			
			if(SetupNextItem())
			{
				// If in mask mode, apply the mask material on the current item as well
				if(ViewModes[CurrViewModeIdx] == ESLItemScannerViewMode::Mask)
				{
					OriginalMaterials.Empty();
					ApplyMaskMaterial();
				}
			
				// No other scan poses found, set next item and first camera scan pose
				SetupFirstScanPose();

				MetadataLoggerParent->StartScanEntry(ScanItems[CurrItemIdx].Value, SizeX, SizeY);
				ScanPoseData.Pose = ScanPoses[CurrPoseIdx];
				
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

// Apply view mode
void USLItemScanner::ApplyViewMode(ESLItemScannerViewMode Mode)
{
	// No change in the rendering view mode
	if(Mode == PrevViewMode)
	{
		return;
	}

	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if(Mode == ESLItemScannerViewMode::Color)
	{
		if(PrevViewMode == ESLItemScannerViewMode::Depth || PrevViewMode == ESLItemScannerViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Mask)
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		ViewModePostfix = "L";
		
		//if(bBufferVisOn)
		//{
		//	ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		//	bBufferVisOn = false;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bBufferVisOn = false "), *FString(__func__), __LINE__);
		//}
		//if(bMaskMatOn)
		//{
		//	ApplyOriginalMaterial();
		//	bMaskMatOn = false;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bMaskMatOn = false "), *FString(__func__), __LINE__);
		//}
		//if(bUnlitOn)
		//{
		//	GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		//	bUnlitOn = false;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bUnlitOn = false "), *FString(__func__), __LINE__);
		//}
		//
		//UE_LOG(LogTemp, Error, TEXT("%s::%d \t ************** L **************"), *FString(__func__), __LINE__);
	}
	else if(Mode == ESLItemScannerViewMode::Unlit)
	{
		if(PrevViewMode == ESLItemScannerViewMode::Color)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Mask)
		{
			ApplyOriginalMaterial();
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Depth || PrevViewMode == ESLItemScannerViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		ViewModePostfix = "U";
		
		//if(bBufferVisOn)
		//{
		//	ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		//	bBufferVisOn = false;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bBufferVisOn = false "), *FString(__func__), __LINE__);
		//}
		//if(bMaskMatOn)
		//{
		//	ApplyOriginalMaterial();
		//	bMaskMatOn = false;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bMaskMatOn = false "), *FString(__func__), __LINE__);
		//}
		//if(!bUnlitOn)
		//{
		//	GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		//	bUnlitOn = true;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bUnlitOn = true "), *FString(__func__), __LINE__);
		//}
		//
		//UE_LOG(LogTemp, Error, TEXT("%s::%d \t ************** U ************** "), *FString(__func__), __LINE__);
	}
	else if(Mode == ESLItemScannerViewMode::Mask)
	{
		if(PrevViewMode == ESLItemScannerViewMode::Unlit)
		{
			ApplyMaskMaterial();
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Color)
		{
			ApplyMaskMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Depth || PrevViewMode == ESLItemScannerViewMode::Normal)
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
		ViewModePostfix = "M";
		
		//if(bBufferVisOn)
		//{
		//	ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		//	bBufferVisOn = false;
		//}
		//if(!bMaskMatOn)
		//{
		//	ApplyMaskMaterial();
		//	bMaskMatOn = true;
		//}
		//if(!bUnlitOn)
		//{
		//	GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		//	bUnlitOn = true;
		//}
		//ViewModePostfix = "M";
	}
	else if(Mode == ESLItemScannerViewMode::Depth)
	{
		if(PrevViewMode == ESLItemScannerViewMode::Mask)
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Normal)
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
		ViewModePostfix = "D";

		
		//if(bMaskMatOn)
		//{
		//	ApplyOriginalMaterial();
		//	bMaskMatOn = false;
		//}
		//if(!bBufferVisOn)
		//{
		//	ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		//	bBufferVisOn = true;
		//}
		//if(!bDepthVisOn)
		//{
		//	BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		//	bDepthVisOn = true;
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t bUnlitOn = true "), *FString(__func__), __LINE__);
		//}
		//ViewModePostfix = "D";
		//UE_LOG(LogTemp, Error, TEXT("%s::%d \t ************** D ************** "), *FString(__func__), __LINE__);
	}
	else if(Mode == ESLItemScannerViewMode::Normal)
	{
		if(PrevViewMode == ESLItemScannerViewMode::Depth)
		{
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Mask)
		{
			ApplyOriginalMaterial();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLItemScannerViewMode::Color)
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
		ViewModePostfix = "N";
		
		//if(bMaskMatOn)
		//{
		//	ApplyOriginalMaterial();
		//	bMaskMatOn = false;
		//}
		//if(!bBufferVisOn)
		//{
		//	ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		//	bBufferVisOn = true;
		//}
		//if(bDepthVisOn)
		//{
		//	BufferVisTargetCV->Set(*FString("WorldNormal"));
		//	bDepthVisOn = false;
		//}
		//ViewModePostfix = "N";
	}

	// Cache as previous view mode
	PrevViewMode = Mode;
}

// Apply mask material to current item
void USLItemScanner::ApplyMaskMaterial()
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
void USLItemScanner::ApplyOriginalMaterial()
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
void USLItemScanner::QuitEditor()
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

/* Helpers */
// Get the bounding box and the number of pixels the item occupies in the image
void USLItemScanner::GetItemPixelNumAndBB(const TArray<FColor>& InBitmap, int32 Width, int32 Height,
	int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax)
{
	if(ViewModes[CurrViewModeIdx] == ESLItemScannerViewMode::Mask)
	{
		GetColorPixelNumAndBB(InBitmap, MaskColorConst, Width, Height,OutPixelNum, OutBBMin, OutBBMax);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Only works in Mask mode.."), *FString(__func__));
	}
}

// Get the bounding box and the number of pixels the color occupies in the image
void USLItemScanner::GetColorPixelNumAndBB(const TArray<FColor>& InBitmap, const FColor& Color, int32 Width,
	int32 Height, int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax)
{
	int32 RowIdx = 0;
	int32 ColIdx = 0;
	OutPixelNum = 0;
	OutBBMin = FIntPoint(Width, Height);
	OutBBMax = FIntPoint(0,0);

	for (const auto& C : InBitmap)
	{
		if(C == Color)
		{
			OutPixelNum++;
			
			if(RowIdx < OutBBMin.Y)
			{
				OutBBMin.Y = RowIdx;
			}
			if(RowIdx > OutBBMax.Y)
			{
				OutBBMax.Y = RowIdx;
			}
			if(ColIdx < OutBBMin.X)
			{
				OutBBMin.X = ColIdx;
			}
			if(ColIdx > OutBBMax.X)
			{
				OutBBMax.X = ColIdx;
			}
		}

		//// DEBUG
		//if(C == Color)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t[\t%ld\t|\t%ld\t];\t\t||\t\tMin=[\t%ld\t|\t%ld\t];\t\t||\t\tMax=[\t%ld\t|\t%ld\t]]"),
		//		*FString(__func__), __LINE__, RowIdx, ColIdx, OutBBMin.X, OutBBMin.Y, OutBBMax.X, OutBBMax.Y);
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t[\t%ld\t|\t%ld\t];\t\t||\t\tMin=[\t%ld\t|\t%ld\t];\t\t||\t\tMax=[\t%ld\t|\t%ld\t]]"),
		//		*FString(__func__), __LINE__, RowIdx, ColIdx, OutBBMin.X, OutBBMin.Y, OutBBMax.X, OutBBMax.Y);
		//}

		// Next column
		ColIdx++;

		// Check for row change
		if(ColIdx > Width - 1)
		{
			ColIdx = 0;
			RowIdx++;
		}
	}
}

// Get the number of pixels that the item occupies in the image
int32 USLItemScanner::GetItemPixelNum(const TArray<FColor>& Bitmap)
{
	if(ViewModes[CurrViewModeIdx] == ESLItemScannerViewMode::Mask)
	{
		return GetColorPixelNum(Bitmap, MaskColorConst);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Only works in Mask mode, returning -1"), *FString(__func__));
		return INDEX_NONE;
	}
}

// Get the number of pixels of the given color in the image
int32 USLItemScanner::GetColorPixelNum(const TArray<FColor>& Bitmap, const FColor& Color) const
{
	int32 Num = 0;
	for (const auto& C : Bitmap)
	{
		if(C == Color)
		{
			Num++;
		}
	}
	return Num++;
}

// Get the number of pixels of the given two colors in the image
void USLItemScanner::GetColorsPixelNum(const TArray<FColor>& Bitmap, const FColor& ColorA, int32& OutNumA, const FColor& ColorB, int32& OutNumB)
{
	OutNumA = 0;
	OutNumB = 0;
	for (const auto& C : Bitmap)
	{
		if(C == ColorA)
		{
			OutNumA++;
		}
		else if(C == ColorB)
		{
			OutNumB++;
		}
	}
}

// Count (and check) the number of pixels the item uses in the image
void USLItemScanner::CountItemPixelNumWithCheck(const TArray<FColor>& Bitmap, int32 ResX, int32 ResY)
{
	// Count pixel colors
	if(ViewModes[CurrViewModeIdx] == ESLItemScannerViewMode::Mask)
	{
		// Cache the previous number of item pixels
		int32 PrevItemPixelNum = TempItemPixelNum;
		
		int32 NumBackgroundPixels = 0;
		GetColorsPixelNum(Bitmap, FColor::Black, NumBackgroundPixels, MaskColorConst, TempItemPixelNum);
		if(TempItemPixelNum + NumBackgroundPixels != ResX * ResY)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Number of item [%ld] + background [%ld] pixels, differs of number image total [%ld]."),
				*FString(__func__), __LINE__, TempItemPixelNum, NumBackgroundPixels, ResX * ResY);
		}

		// Compare against previous (other view mode) number item pixels
		if(PrevItemPixelNum != INDEX_NONE)
		{
			if(TempItemPixelNum != PrevItemPixelNum)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Prev number [%ld] of item pixels differs of current one [%ld]."),
					*FString(__func__), __LINE__, PrevItemPixelNum, TempItemPixelNum);
			}
		}
	}
	else if(ViewModes[CurrViewModeIdx] == ESLItemScannerViewMode::Unlit)
	{
		// Cache the previous number of item pixels
		int32 PrevItemPixelNum = TempItemPixelNum;
		
		int32 NumBackgroundPixels = GetColorPixelNum(Bitmap, FColor::Black);
		TempItemPixelNum = ResX * ResY - NumBackgroundPixels;

		// Compare against previous (other view mode) number item pixels
		if(PrevItemPixelNum != INDEX_NONE)
		{
			if(TempItemPixelNum != PrevItemPixelNum)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Prev number [%ld] of item pixels differs of current one [%ld]."),
					*FString(__func__), __LINE__, PrevItemPixelNum, TempItemPixelNum);
			}
		}
	}
}

// Get the view mode string name
FString USLItemScanner::GetViewModeName(ESLItemScannerViewMode Mode) const
{
	if(Mode == ESLItemScannerViewMode::Color)
	{
		return FString("Color");
	}
	else if(Mode == ESLItemScannerViewMode::Unlit)
	{
		return FString("Unlit");
	}
	else if(Mode == ESLItemScannerViewMode::Mask)
	{
		return FString("Mask");
	}
	else if(Mode == ESLItemScannerViewMode::Depth)
	{
		return FString("Depth");
	}
	else if(Mode == ESLItemScannerViewMode::Normal)
	{
		return FString("Normal");
	}
	else
	{
		return FString("None");
	}
}

// Check if the items meets the requirements to be scanned
float USLItemScanner::GetItemRelativeCameraDistance(UStaticMeshComponent* SMC) const
{
	return SMC->Bounds.SphereRadius * 1.5f;
}

// Check against varios properties if the item should be scanned
bool USLItemScanner::HasScanningRequirements(UStaticMeshComponent* SMC, float MaxVolume, float MaxBoundsLength,
	bool bWithSemanticContactShape, bool bOnlyMovable) const
{
	bool bShouldBeScanned = true;
	if(bOnlyMovable && SMC->Mobility != EComponentMobility::Movable)
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t %s is not movable, skipping item scan.."),
		//	*FString(__func__), __LINE__, *SMC->GetOwner()->GetName());
		return false;
	}

	if(bWithSemanticContactShape && !HasSemanticContactShape(SMC))
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

// Check if the item is wrapped in a semantic contact shape (has a SLContactShapeInterface sibling)
bool USLItemScanner::HasSemanticContactShape(UStaticMeshComponent* SMC) const
{
	for(const auto C : SMC->GetOwner()->GetComponentsByClass(UShapeComponent::StaticClass()))
	{
		if (Cast<ISLContactShapeInterface>(C))
		{
			return true;
		}
	}
	return false;
}

// Generate sphere scan poses
void USLItemScanner::GenerateSphereScanPoses(uint32 MaxNumOfPoints, float Radius, TArray<FTransform>& OutTransforms)
{
	// (https://www.cmu.edu/biolphys/deserno/pdf/sphere_equi.pdf)
	const float Area = 4 * PI / MaxNumOfPoints;
	const float Distance = FMath::Sqrt(Area);

	// Num of latitudes
	const int32 MTheta = FMath::RoundToInt(PI/Distance);
	const float DTheta = PI / MTheta;
	const float DPhi = Area / DTheta;

	// Iterate latitude lines
	for (int32 M = 0; M < MTheta; M++)
	{
		// 0 <= Theta <= PI
		const float Theta = PI * (float(M) + 0.5) / MTheta;

		// Num of longitudes
		const int32 MPhi = FMath::RoundToInt(2 * PI * FMath::Sin(Theta) / DPhi);
		for (int32 N = 0; N < MPhi; N++)
		{
			// 0 <= Phi < 2pi 
			const float Phi = 2 * PI * N / MPhi;

			FVector Point;
			Point.X = FMath::Sin(Theta) * FMath::Cos(Phi) * Radius;
			Point.Y = FMath::Sin(Theta) * FMath::Sin(Phi) * Radius;
			Point.Z = FMath::Cos(Theta) * Radius;
			FQuat Quat = (-Point).ToOrientationQuat();

			OutTransforms.Emplace(Quat, Point);
		}
	}
}
