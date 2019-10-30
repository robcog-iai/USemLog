// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLItemScanner.h"
#include "SLEntitiesManager.h"
#include "SLContactShapeInterface.h"
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

// Ctor
USLItemScanner::USLItemScanner()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bUnlit = false;
	bIncludeLocally = false;
	CurrViewModeIdx = INDEX_NONE;
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
		 FIntPoint Resolution, const TSet<ESLItemScannerViewMode>& InViewModes, bool bIncludeScansLocally, bool bOverwrite)
{
	if (!bIsInit)
	{
		Location = InTaskId;
		ViewModes = InViewModes.Array();
		bIncludeLocally = bIncludeScansLocally;

		// If no view modes are available, add a default one
		if(ViewModes.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No view modes found, added default one (lit).."), *FString(__func__), __LINE__);
			ViewModes.Add(ESLItemScannerViewMode::Lit);
		}
		else
		{
			if (ViewModes.Contains(ESLItemScannerViewMode::Mask))
			{
				if(!LoadMaskMaterial())
				{
					ViewModes.Remove(ESLItemScannerViewMode::Mask);
				}
			}
		}

		// Spawn helper actors, load scan items and points
		if(!LoadScanBoxActor() || !LoadScanCameraPoseActor() || !LoadScanPoints() || !LoadScanItems(true, true))
		{
			return;
		}

		// Used for the screenshot requests
		ViewportClient = GetWorld()->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLItemScanner::ScreenshotCB);

		// Set scan image resolution and various rendering paramaters
		InitRenderParameters(Resolution);

		// Disable physics on all actors
		for (TActorIterator<AActor> Act(GetWorld()); Act; ++Act)
		{
			Act->DisableComponentsSimulatePhysics();
		}
		
		bIsInit = true;
	}
}

// Start scanning
void USLItemScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
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

		// Cannot be called in Init() because GetFirstPlayerController() is nullptr before BeginPlay
		SetupFirstViewMode();

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

// Load scan box actor
bool USLItemScanner::LoadScanBoxActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanBox");
	ScanBoxActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
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
bool USLItemScanner::LoadScanItems(bool bWithMask, bool bWithContactShape)
{
	// Cache the classes that were already iterated
	TSet<FString> IteratedClasses;
	
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
	{
		const FSLEntity SemanticData = Pair.Value;

		// Skip if class was already checked
		if(IteratedClasses.Contains(SemanticData.Class))
		{
			continue;
		}
		IteratedClasses.Emplace(SemanticData.Class);
		
		// Check if the item has a visual
		if (AStaticMeshActor* ObjAsSMA = Cast<AStaticMeshActor>(SemanticData.Obj))
		{
			if(UStaticMeshComponent* SMC = ObjAsSMA->GetStaticMeshComponent())
			{
				if(IsHandheldItem(SMC))
				{
					if(bWithMask && !SemanticData.HasVisualMask())
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %s has no visual mask, skipping item scan.."),
							*FString(__func__), __LINE__, *SemanticData.Class);
						continue;
					}
					
					if(bWithContactShape && !HasSemanticContactShape(SMC))
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t %s has no semantic contact shape, skipping item scan.."),
							*FString(__func__), __LINE__, *SemanticData.Class);
						continue;
					}
					
					SMC->SetSimulatePhysics(false);
					ScanItems.Emplace(SMC, SemanticData);
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Added %s to the scan items list.."),
						*FString(__func__), __LINE__, *SemanticData.Class);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d \t %s is not a handheld item (too large, not movable), skipping item scan.."),
						*FString(__func__), __LINE__, *SemanticData.Class);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no static mesh component, skipping item scan.."),
					*FString(__func__), __LINE__, *SemanticData.Class);
			}
		}
		else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(SemanticData.Obj))
		{
			if(IsHandheldItem(ObjAsSMC))
			{
				if(bWithContactShape && !HasSemanticContactShape(ObjAsSMC))
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t %s has no semantic contact shape, skipping item scan.."),
						*FString(__func__), __LINE__, *SemanticData.Class);
					continue;
				}

				if(bWithMask && !SemanticData.HasVisualMask())
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t %s has no visual mask, skipping item scan.."),
						*FString(__func__), __LINE__, *SemanticData.Class);
					continue;
				}
				
				ObjAsSMC->SetSimulatePhysics(false);
				ScanItems.Emplace(ObjAsSMC, SemanticData);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Added %s to the scan items list.."),
						*FString(__func__), __LINE__, *SemanticData.Class);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t %s is not a handheld item (too large, not movable), skipping item scan.."),
					*FString(__func__), __LINE__, *SemanticData.Class);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not a static mesh actor, skipping scan.."),
				*FString(__func__), __LINE__, *SemanticData.Class);
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
	// Move the first item to the center of the scan box
	CurrItemIdx = 0;
	if(!ScanItems.IsValidIndex(CurrItemIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid item index [%ld].."), *FString(__func__), __LINE__, CurrItemIdx);
		return false;
	}
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorLocation(ScanBoxActor->GetActorLocation());
	return true;
}

// Set next item in the scan box, return false if there are no more items
bool USLItemScanner::SetupNextItem()
{
	// Move previous item away
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorLocation(FVector(0.f));
	
	// Bring next item
	CurrItemIdx++;
	if(!ScanItems.IsValidIndex(CurrItemIdx))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Last item [%ld] reached.."), *FString(__func__), __LINE__, CurrItemIdx-1);
		return false;
	}
	ScanItems[CurrItemIdx].Key->GetOwner()->SetActorLocation(ScanBoxActor->GetActorLocation());
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
	CameraPoseActor->SetActorTransform(ScanPoses[CurrPoseIdx]);
	GetWorld()->GetFirstPlayerController()->SetViewTarget(CameraPoseActor);
	return true;
}

// Request a screenshot
void USLItemScanner::RequestScreenshot()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		CurrScanName = FString::FromInt(CurrItemIdx) + "_" + ScanItems[CurrItemIdx].Value.Class + "_" + FString::FromInt(CurrPoseIdx);
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
	UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Progress=[Item=%ld/%ld; Pose=%ld/%ld; ViewMode=%ld/%ld; Scan=%ld/%ld]"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
		CurrItemIdx + 1, ScanItems.Num(),
		CurrPoseIdx + 1, ScanPoses.Num(),
		CurrViewModeIdx + 1, ViewModes.Num(),
		CurrItemIdx * ScanPoses.Num() * ViewModes.Num() + CurrPoseIdx * ViewModes.Num() + CurrViewModeIdx + 1,
		ScanItems.Num() * ScanPoses.Num() * ViewModes.Num());

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

	if(SetupNextViewMode())
	{
		RequestScreenshot();
	}
	else
	{
		if(SetupFirstViewMode())
		{
			if(SetupNextScanPose())
			{
				RequestScreenshot();
			}
			else if(SetupNextItem())
			{
				SetupFirstScanPose();
				SetupFirstViewMode();
				RequestScreenshot();
			}
			else
			{
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
	if(Mode == ESLItemScannerViewMode::Lit)
	{
		ApplyOriginalMaterial();
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		ViewModePostfix = "L";
	}
	else if(Mode == ESLItemScannerViewMode::Unlit)
	{
		ApplyOriginalMaterial();
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		ViewModePostfix = "U";
	}
	else if(Mode == ESLItemScannerViewMode::Mask)
	{
		ApplyMaskMaterial();
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		ViewModePostfix = "M";
	}
}

// Apply mask material to current item
void USLItemScanner::ApplyMaskMaterial()
{
	if(DynamicMaskMaterial)
	{
		UStaticMeshComponent* SMC = ScanItems[CurrItemIdx].Key;
		OriginalMaterials = SMC->GetMaterials();
		
		FColor MaskColor(FColor::FromHex(ScanItems[CurrItemIdx].Value.VisualMask));
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));

		for (int32 Idx = 0; Idx < SMC->GetNumMaterials(); ++Idx)
		{
			SMC->SetMaterial(Idx, DynamicMaskMaterial);
		}
	}
}

// Apply original material to current item
void USLItemScanner::ApplyOriginalMaterial()
{
	if(OriginalMaterials.Num() > 0)
	{
		int32 Idx = 0;
		for(const auto& M : OriginalMaterials)
		{
			ScanItems[CurrItemIdx].Key->SetMaterial(Idx, M);
		}
		OriginalMaterials.Empty();
	}
}

// Check if the items meets the requirements to be scanned
bool USLItemScanner::IsHandheldItem(UStaticMeshComponent* SMC) const
{
	return SMC->Mobility == EComponentMobility::Movable &&
		SMC->Bounds.GetBox().GetVolume() < VolumeLimit &&
		SMC->Bounds.SphereRadius * 2.f < LengthLimit;
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