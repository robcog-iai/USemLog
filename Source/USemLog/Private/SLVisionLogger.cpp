// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionLogger.h"
#include "Vision/SLVisionPoseableMeshActor.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"

// Constructor
USLVisionLogger::USLVisionLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false), bIsPaused(false)
{
	CurrViewModeIdx = INDEX_NONE;
	CurrVirtualCameraIdx = INDEX_NONE;
	CurrTimestamp = -1.f;
	PrevViewMode = ESLVisionViewMode::NONE;

	ViewModes.Add(ESLVisionViewMode::Color);
	ViewModes.Add(ESLVisionViewMode::Unlit);
	ViewModes.Add(ESLVisionViewMode::Mask);
	ViewModes.Add(ESLVisionViewMode::Depth);
	ViewModes.Add(ESLVisionViewMode::Normal);
}

// Destructor
USLVisionLogger::~USLVisionLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}

	// Disconnect and clean db connection
	DBHandler.Disconnect();
}

// Init Logger
void USLVisionLogger::Init(const FString& InTaskId, const FString& InEpisodeId, const FString& InServerIp, uint16 InServerPort,
	bool bOverwriteVisionData, const FSLVisionLoggerParams& Params)
{
	if (!bIsInit)
	{
		Resolution = Params.Resolution;

		// Save the folder name if the images are going to be stored locally as well
		if(Params.bIncludeLocally)
		{
			SaveLocallyFolderName = InTaskId + "/" + InEpisodeId + "_Vis";
		}

		// Set the captured image resolution
		InitScreenshotResolution(Resolution);

		// Set rendering parameters
		InitRenderParameters();

		// Disable physics on all entities and make sure they are movable
		InitWorldEntities();

		// Create movable clones of the skeletal meshes, hide originals (call before loading the episode data)
		CreatePoseableMeshesClones();

		// Connect to the database for writing the image data
		if (!DBHandler.Connect(InTaskId, InEpisodeId, InServerIp, InServerPort, bOverwriteVisionData))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not connect to the DB.."), *FString(__func__), __LINE__);
			return;
		}

		// Download the whole episode data (make sure the poseable mesh clones are created before this)
		if (!DBHandler.GetEpisodeData(Params.UpdateRate, SkelToPoseableMap, Episode))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not download the episode data.."), *FString(__func__), __LINE__);
			return;
		}

		// Make sure rendering modes are selected
		if(ViewModes.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No view modes found.."), *FString(__func__), __LINE__);
			return;
		}

		// Get access to the virtual cameras (make sure the semantic entities are init before)
		if(!LoadVirtualCameras())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No virtual cameras found.."), *FString(__func__), __LINE__);
			return;
		}

		// Access the viewport (used for the screenshot requests)
		ViewportClient = GetWorld()->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}
		// Bind the screenshot captured callback
		ScreenshotCallbackHandle = ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLVisionLogger::ScreenshotCB);

		// Create clones of every visible entity with a mask color
		if(ViewModes.Contains(ESLVisionViewMode::Mask))
		{
			if(CreateMaskClones())
			{
				// Create color to semantic data mappings on the image handler, setup the rendered to original mask mapping
				if (!MaskImgHandler.Init())
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init image handler, removing mask view type.."), *FString(__func__), __LINE__);
					ViewModes.Remove(ESLVisionViewMode::Mask);
				}
				
				// Actors that need to be hidden in mask mode (skylight, fog etc.)
				//MaskViewModeBlacklistedActors = FSLEntitiesManager::GetInstance()->GetUntaggedActors();

				if (Params.bCalculateOverlaps)
				{
					// Create the overlap calc object
					OverlapCalc = NewObject<USLVisionOverlapCalc>(this);
					// Give control to the overlap calc to pause and start the vision logger
					OverlapCalc->Init(this, Resolution/Params.OverlapResolutionDivisor, SaveLocallyFolderName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create mask clones, removing mask view type.."), *FString(__func__), __LINE__);
				ViewModes.Remove(ESLVisionViewMode::Mask);
			}


		}
		bIsInit = true;
	}
}

// Start logger
void USLVisionLogger::Start(const FString& EpisodeId)
{
	if (!bIsStarted && bIsInit)
	{
		// Hide default pawn from scene
		GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);		
		
		// Setup the first frame, camera and view mode
		if (FirstStep())
		{
			// Init data
			CurrFrameData.Init(CurrTimestamp, Resolution);
			CurrViewData.Init(VirtualCameras[CurrVirtualCameraIdx]->GetId(), VirtualCameras[CurrVirtualCameraIdx]->GetClassName());

			// Start recursion
			RequestScreenshot();
			bIsStarted = true;
		}		
	}
}

// Finish logger
void USLVisionLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Index the entries in the db
		DBHandler.CreateIndexes();

		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsPaused = false;
		bIsFinished = true;		
	}
}

// Can be called if init
void USLVisionLogger::Pause(bool Value)
{
	if (bIsStarted)
	{
		if (bIsPaused != Value)
		{
			if (Value)
			{
				// Remove the screenshot callback
				ViewportClient->OnScreenshotCaptured().Remove(ScreenshotCallbackHandle);

				bIsPaused = true;
			}
			else
			{
				// Re-bind the screenshot captured callback
				ScreenshotCallbackHandle = ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLVisionLogger::ScreenshotCB);

				// Set the captured image resolution
				InitScreenshotResolution(Resolution);	

				// Go to next frame/camera/view mode
				if (NextStep())
				{
					RequestScreenshot();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Finished visual logger.."),
						*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
					QuitEditor();
				}

				bIsPaused = false;
			}
		}
	}
}

// Get access to the static mesh clone from the id
AStaticMeshActor* USLVisionLogger::GetStaticMeshMaskCloneFromId(const FString& Id)
{
	//if (AStaticMeshActor* SMA = FSLEntitiesManager::GetInstance()->GetStaticMeshActor(Id))
	//{
	//	if (AStaticMeshActor** SMAClone = OrigToMaskClones.Find(SMA))
	//	{
	//		return *SMAClone;
	//	}
	//}
	return nullptr;
}

// Get access to the poseable skeletal mesh clone from the id
ASLVisionPoseableMeshActor* USLVisionLogger::GetPoseableSkeletalMaskCloneFromId(const FString& Id, USLSkeletalDataComponent** OutSkelDataComp)
{
	//if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor(Id))
	//{
	//	if (OutSkelDataComp != nullptr)
	//	{
	//		*OutSkelDataComp = CastChecked<USLSkeletalDataComponent>(SkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()));
	//	}

	//	if (ASLVisionPoseableMeshActor** PMAClone = SkelToPoseableMap.Find(SkMA))
	//	{
	//		if (ASLVisionPoseableMeshActor** PMAMaskClone = PoseableOrigToMaskClones.Find(*PMAClone))
	//		{
	//			return *PMAMaskClone;
	//		}
	//	}
	//}
	return nullptr;
}

// Trigger the screenshot on the game thread
void USLVisionLogger::RequestScreenshot()
{
	//// FrameNum_CameraName_ViewMode
	//CurrImageFilename = FString::FromInt(EpisodeDataRaw.GetActiveFrameNum()) + "_" + 
	//	VirtualCameras[CurrCameraIdx]->GetClassName() + "_" + CurrViewModePostfix;

	// FrameNum_CameraNum_s-ms_Viewmode
	//CurrImageFilename = FString::FromInt(Episode.GetCurrIndex()) + "_" + 
	//	FString::FromInt(CurrVirtualCameraIdx) + "_" + 
	//	FString::SanitizeFloat(CurrTimestamp).Replace(TEXT("."),TEXT("-")) + "_" + CurrViewModePostfix;

	// FrameNum_CameraNum_Viewmode
	//CurrImageFilename = FString::FromInt(Episode.GetCurrIndex()) + "_" + 
	//	FString::FromInt(CurrVirtualCameraIdx) + "_" + CurrViewModePostfix;

	// Sec-Ms_FrameNum_Viewmode
	CurrImageFilename = FString::Printf(TEXT("%.2f"), CurrTimestamp).Replace(TEXT("."), TEXT("-")) + "_" +
		FString::FromInt(Episode.GetCurrIndex()) + "_" + CurrViewModePostfix;

	GetHighResScreenshotConfig().FilenameOverride = CurrImageFilename;
	//GetHighResScreenshotConfig().SetForce128BitRendering(true);
	//GetHighResScreenshotConfig().SetHDRCapture(true);

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when the screenshot is captured
void USLVisionLogger::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	// Terminal output with the log progress
	PrintProgress();

	// Compress image
	TArray<uint8> CompressedBitmap;

	// If mask mode is currently active, restore the colors and get the entity data
	if (ViewModes[CurrViewModeIdx] == ESLVisionViewMode::Mask)
	{
		// Remove const-ness from image (needed for restore the image masks from the rendered values to the original ones)
		TArray<FColor>& BitmapRef = const_cast<TArray<FColor>&>(Bitmap);

		// Get information from the mask image and restore any rendering artefacts to the original mask colors
		MaskImgHandler.GetDataAndRestoreImage(BitmapRef, SizeX, SizeY, CurrViewData);

		// Compress the restored bitmap image
		FImageUtils::CompressImageArray(SizeX, SizeY, BitmapRef, CompressedBitmap);
	
		if (OverlapCalc)
		{
			// Check if the image should be saved locally as well
			if (!SaveLocallyFolderName.IsEmpty())
			{
				SaveImageLocally(CompressedBitmap);
			}

			// Cache the image binary
			CurrViewData.Images.Emplace(FSLVisionImageData(GetViewModeName(ViewModes[CurrViewModeIdx]), CompressedBitmap));

			// Bind the screenshot callback for calculating overlaps
			OverlapCalc->Start(&CurrViewData, CurrTimestamp, Episode.GetCurrIndex());

			// Wait for next step until the overlaps were calculated
			return;
		}
	}
	else
	{
		// Compress the original bitmap image
		FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);
	}

	// Check if the image should be saved locally as well
	if(!SaveLocallyFolderName.IsEmpty())
	{
		SaveImageLocally(CompressedBitmap);
	}

	// Cache the image binary
	CurrViewData.Images.Emplace(FSLVisionImageData(GetViewModeName(ViewModes[CurrViewModeIdx]), CompressedBitmap));

	// Go to next frame/camera/view mode
	if (NextStep())
	{
		RequestScreenshot();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Finished visual logger.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		QuitEditor();
	}
}

// Start the dominoes, setup the first frame, camera and view mode, return true if succesfull
bool USLVisionLogger::FirstStep()
{
	// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr
	if (!SetupFirstEpisodeFrame())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup the first world frame.."), *FString(__func__), __LINE__);
		return false;
	}

	// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr at this point
	if (!GotoFirstCameraView())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup first camera view.."), *FString(__func__), __LINE__);
		return false;
	}

	// Set the first render type
	if (!SetupFirstViewMode())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup first view mode.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

// Proceed to the next step (frame, camera, or view mode), return true if successfull
bool USLVisionLogger::NextStep()
{
	if (SetupNextViewMode())
	{
		return true;
	}
	else
	{
		// Current view is processed, cache the data
		CurrFrameData.Views.Emplace(CurrViewData);

		SetupFirstViewMode();

		if (GotoNextCameraView())
		{
			// Start a new view data
			CurrViewData.Clear();
			CurrViewData.Init(VirtualCameras[CurrVirtualCameraIdx]->GetId(), VirtualCameras[CurrVirtualCameraIdx]->GetClassName());

			return true;
		}
		else
		{
			// Write vision frame data to the database
			DBHandler.WriteFrame(CurrFrameData);

			if (SetupNextEpisodeFrame())
			{
				GotoFirstCameraView();

				CurrViewData.Clear();
				CurrViewData.Init(VirtualCameras[CurrVirtualCameraIdx]->GetId(), VirtualCameras[CurrVirtualCameraIdx]->GetClassName());

				CurrFrameData.Clear();
				CurrFrameData.Init(CurrTimestamp, Resolution);

				return true;
			}
			else
			{
				// Last episode frame, with the last camera location and the last view mode was proccessed
				return false;
			}
		}
	}
}

// Goto the first episode frame
bool USLVisionLogger::SetupFirstEpisodeFrame()
{
	if(!Episode.SetupFirstFrame(CurrTimestamp, true, OrigToMaskClones, PoseableOrigToMaskClones))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d First frame not available.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Goto next episode frame, return false if there are no other left
bool USLVisionLogger::SetupNextEpisodeFrame()
{
	if(!Episode.SetupNextFrame(CurrTimestamp, true, OrigToMaskClones, PoseableOrigToMaskClones))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d No new frames.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Goto the first virtual camera view
bool USLVisionLogger::GotoFirstCameraView()
{
	CurrVirtualCameraIdx = 0;
	if(!VirtualCameras.IsValidIndex(CurrVirtualCameraIdx))
	{
		CurrVirtualCameraIdx = INDEX_NONE;
		return false;
	}
	
	GetWorld()->GetFirstPlayerController()->SetViewTarget(VirtualCameras[CurrVirtualCameraIdx]);
	return true;

}

// Goto next camera view, return false if there are no other
bool USLVisionLogger::GotoNextCameraView()
{
	CurrVirtualCameraIdx++;
	if(!VirtualCameras.IsValidIndex(CurrVirtualCameraIdx))
	{
		CurrVirtualCameraIdx = INDEX_NONE;
		return false;
	}

	GetWorld()->GetFirstPlayerController()->SetViewTarget(VirtualCameras[CurrVirtualCameraIdx]);
	return true;
}

// Setup first view mode (render type)
bool USLVisionLogger::SetupFirstViewMode()
{
	CurrViewModeIdx = 0;
	if(!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		return false;
	}
	
	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Setup next view mode (render type), return false if there are no other left
bool USLVisionLogger::SetupNextViewMode()
{
	CurrViewModeIdx++;
	if(!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		CurrViewModeIdx = INDEX_NONE;
		return false;
	}

	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Create movable clones of the skeletal meshes, hide originals (call before loading the episode data)
void USLVisionLogger::CreatePoseableMeshesClones()
{
	TArray<ASkeletalMeshActor*> SkeletalActors;
	//FSLEntitiesManager::GetInstance()->GetSkeletalMeshActors(SkeletalActors);
	for(const auto& SkMA : SkeletalActors)
	{
		// Create a custom actor with a poseable mesh component
		FActorSpawnParameters SpawnParams;
		const FString LabelName = FString(TEXT("PMA_")).Append(SkMA->GetName());
		SpawnParams.Name = FName(*LabelName);
		ASLVisionPoseableMeshActor* PMA = GetWorld()->SpawnActor<ASLVisionPoseableMeshActor>(
			ASLVisionPoseableMeshActor::StaticClass(), SpawnParams);
#if WITH_EDITOR
		PMA->SetActorLabel(LabelName);
#endif // WITH_EDITOR

		if(PMA->Init(SkMA))
		{
			// Add actor to the quick access map
			SkelToPoseableMap.Emplace(SkMA, PMA);
		}

		// Hide original actor
		SkMA->SetActorHiddenInGame(true);
	}
}

// Load the pointers to the virtual cameras
bool USLVisionLogger::LoadVirtualCameras()
{
	//FSLEntitiesManager::GetInstance()->GetCameraViewsObjects(VirtualCameras);
	return VirtualCameras.Num() > 0;
}

// Disable all actors physics and set them to movable
void USLVisionLogger::InitWorldEntities()
{
	// Disable physics on all actors
	for (TActorIterator<AActor> Act(GetWorld()); Act; ++Act)
	{
		Act->DisableComponentsSimulatePhysics();
		if(Act->GetRootComponent())
		{
			Act->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			// TODO check if static lights should be skipped
			Act->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s does not need to be detached.."), *FString(__func__), __LINE__, *Act->GetName());
		}
	}
}

// Create clones of the items with mask material on top
bool USLVisionLogger::CreateMaskClones()
{
	// Load the default mask material
	// this will be used as a template to create the colored mask materials to add to the clones
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this,
		TEXT("/USemLog/CV/M_SLDefaultMask.M_SLDefaultMask"));
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."), *FString(__func__), __LINE__);
		return false;
	}

	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	/* Static meshes */
	TArray<AStaticMeshActor*> SMActors;
	//FSLEntitiesManager::GetInstance()->GetStaticMeshActors(SMActors);
	for(const auto& SMA : SMActors)
	{
		if(UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			//// Get the mask color, will be black if it does not exist
			//FColor SemColor(FColor::FromHex(FTags::GetValue(SMA, "SemLog", "VisMask")));
			//if(SemColor == FColor::Black)
			//{
			//	UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask, setting to black.."), *FString(__func__), __LINE__, *SMA->GetName());
			//}
			//
			//// Create the mask material with the color
			//UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
			//DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));

			//// Create the mask clone 
			//FActorSpawnParameters Parameters;
			//Parameters.Template = SMA;
			//Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			//Parameters.Name = FName(*(SMA->GetName() + TEXT("_MaskClone")));
			//AStaticMeshActor* SMAClone =  GetWorld()->SpawnActor<AStaticMeshActor>(SMA->GetClass(), Parameters);

			//// Apply the generated mask material to the clone
			//if(UStaticMeshComponent* CloneSMC = SMAClone->GetStaticMeshComponent())
			//{
			//	for (int32 MatIdx = 0; MatIdx < CloneSMC->GetNumMaterials(); ++MatIdx)
			//	{
			//		CloneSMC->SetMaterial(MatIdx, DynamicMaskMaterial);
			//	}
			//}

			//// Hide and store the clone
			//SMAClone->SetActorHiddenInGame(true);
			//OrigToMaskClones.Emplace(SMA, SMAClone);
		}
	}

	/* Skeletal meshes */
	TArray<ASkeletalMeshActor*> SkMActors;
	//FSLEntitiesManager::GetInstance()->GetSkeletalMeshActors(SkMActors);
	//for(const auto& SkMA : SkMActors)
	//{
	//	// Get the semantic data component containing the semantics (class names mask colors) about the bones
	//	if (UActorComponent* AC = SkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
	//	{
	//		// Create a custom actor with a poseable mesh component
	//		FActorSpawnParameters SpawnParams;
	//		const FString LabelName = FString(TEXT("PMA_")).Append(SkMA->GetName()).Append("_MaskClone");
	//		SpawnParams.Name = FName(*LabelName);
	//		ASLVisionPoseableMeshActor* PMAClone = GetWorld()->SpawnActor<ASLVisionPoseableMeshActor>(
	//			ASLVisionPoseableMeshActor::StaticClass(), SpawnParams);
	//		PMAClone->SetActorLabel(LabelName);

	//		if (!PMAClone->Init(SkMA))
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init the poseable mesh actor %s.."),
	//				*FString(__func__), __LINE__, *PMAClone->GetName());
	//			PMAClone->Destroy();
	//			continue;
	//		}

	//		// Apply mask materials
	//		USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
	//		for (auto& Pair : SkDC->SemanticBonesData)
	//		{
	//			// Check if bone class and visual mask is set
	//			if(Pair.Value.IsClassSet())
	//			{
	//				// Get the mask color, will be black if it does not exist
	//				FColor SemColor(FColor::FromHex(Pair.Value.VisualMask));
	//				if (SemColor == FColor::Black)
	//				{
	//					UE_LOG(LogTemp, Error, TEXT("%s::%d %s --> %s has no visual mask, setting to black.."),
	//						*FString(__func__), __LINE__, *SkMA->GetName(), *Pair.Value.Class);
	//				}

	//				// Create the mask material with the color
	//				UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
	//				DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
	//					FLinearColor::FromSRGBColor(FColor::FromHex(Pair.Value.VisualMask)));

	//				PMAClone->SetCustomMaterial(Pair.Value.MaterialIndex, DynamicMaskMaterial);
	//			}
	//		}

	//		// Hide and store the skeletal clone
	//		PMAClone->SetActorHiddenInGame(true);
	//		if(ASLVisionPoseableMeshActor** PMAOrig = SkelToPoseableMap.Find(SkMA))
	//		{
	//			PoseableOrigToMaskClones.Add(*PMAOrig, PMAClone);
	//		}
	//		else
	//		{
	//			PMAClone->Destroy();
	//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no poseable mesh conterpart, mask clone will not be used.."),
	//				*FString(__func__), __LINE__, *SkMA->GetName());
	//		}
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
	//			*FString(__func__), __LINE__, *SkMA->GetName());
	//	}
	//}
	return true;
}

// Init hi-res screenshot resolution
void USLVisionLogger::InitScreenshotResolution(FIntPoint InResolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(InResolution.X, InResolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;	
}

// Init render parameters (resolution, view mode)
void USLVisionLogger::InitRenderParameters()
{
	// Defines the memory layout used for the GBuffer,
	// 0: lower precision (8bit per component, for profiling), 1: low precision (default)
	// 3: high precision normals encoding, 5: high precision
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.GBufferFormat"))->Set(5);

	
	// Set the near clipping plane (in cm)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetNearClipPlane"))->Set(0); // Not a console variable, but a command
	//GNearClippingPlane = 0; // View is distorted after finishing the scanning
#if WITH_EDITOR
	if(GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("r.SetNearClipPlane 0"));
	}
#endif // WITH_EDITOR
	
	//// AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_None);

	//// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// LOD level to force, -1 is off. (0 - Best)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Apply view mode
void USLVisionLogger::ApplyViewMode(ESLVisionViewMode Mode)
{
	// No change in the rendering view mode
	if(Mode == PrevViewMode)
	{
		return;
	}

	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if(Mode == ESLVisionViewMode::Color) // viewmode=lit, buffer=false, materials=original;
	{
		if(PrevViewMode == ESLVisionViewMode::Depth || PrevViewMode == ESLVisionViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else if(PrevViewMode == ESLVisionViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else if(PrevViewMode == ESLVisionViewMode::Mask)
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		CurrViewModePostfix = "C";
	}
	else if(Mode == ESLVisionViewMode::Unlit) // viewmode=unlit, buffer=false, materials=original;
	{
		if(PrevViewMode == ESLVisionViewMode::Color)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLVisionViewMode::Mask)
		{
			ApplyOriginalMaterials();
		}
		else if(PrevViewMode == ESLVisionViewMode::Depth || PrevViewMode == ESLVisionViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		CurrViewModePostfix = "U";
	}
	else if(Mode == ESLVisionViewMode::Mask) // viewmode=unlit, buffer=false, materials=mask;
	{
		if(PrevViewMode == ESLVisionViewMode::Unlit)
		{
			ApplyMaskMaterials();
		}
		else if(PrevViewMode == ESLVisionViewMode::Color)
		{
			ApplyMaskMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLVisionViewMode::Depth || PrevViewMode == ESLVisionViewMode::Normal)
		{
			ApplyMaskMaterials();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ApplyMaskMaterials();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		CurrViewModePostfix = "M";
	}
	else if(Mode == ESLVisionViewMode::Depth) // viewmode=unlit, buffer=false, materials=original;
	{
		if(PrevViewMode == ESLVisionViewMode::Mask)
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLVisionViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLVisionViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLVisionViewMode::Normal)
		{
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		CurrViewModePostfix = "D";
	}
	else if(Mode == ESLVisionViewMode::Normal) // viewmode=lit, buffer=true, materials=original;
	{
		if(PrevViewMode == ESLVisionViewMode::Depth)
		{
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLVisionViewMode::Mask)
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLVisionViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLVisionViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		CurrViewModePostfix = "N";
	}

	// Cache as previous view mode
	PrevViewMode = Mode;
}

// Apply mask materials 
void USLVisionLogger::ApplyMaskMaterials()
{
	for(const auto& Pair : OrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(true);
		Pair.Value->SetActorHiddenInGame(false);
	}
	for (const auto& Pair : PoseableOrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(true);
		Pair.Value->SetActorHiddenInGame(false);
	}
	for (const auto& Act : MaskViewModeBlacklistedActors)
	{
		Act->SetActorHiddenInGame(true);
	}
}

// Apply original material to current item
void USLVisionLogger::ApplyOriginalMaterials()
{
	for(const auto& Pair : OrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(false);
		Pair.Value->SetActorHiddenInGame(true);
	}
	for (const auto& Pair : PoseableOrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(false);
		Pair.Value->SetActorHiddenInGame(true);
	}
	for (const auto& Act : MaskViewModeBlacklistedActors)
	{
		Act->SetActorHiddenInGame(false);
	}
}

// Clean exit, all the Finish() methods will be triggered
void USLVisionLogger::QuitEditor()
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
void USLVisionLogger::SaveImageLocally(const TArray<uint8>& CompressedBitmap)
{
	const FString FolderName = VirtualCameras[CurrVirtualCameraIdx]->GetClassName() + "_" + CurrViewModePostfix;
	FString Path = FPaths::ProjectDir() + "/SemLog/" + SaveLocallyFolderName + "/" + FolderName + "/" + CurrImageFilename + ".png";
	FPaths::RemoveDuplicateSlashes(Path);
	FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
}

// Output progress to terminal
void USLVisionLogger::PrintProgress() const
{
	const int32 CurrFrameNr = Episode.GetCurrIndex() + 1;
	const int32 TotalFrames = Episode.GetFramesNum();
	const int32 CurrCameraNr = CurrVirtualCameraIdx + 1;
	const int32 TotalCameras = VirtualCameras.Num();
	const int32 CurrViewModeNr = CurrViewModeIdx + 1;
	const int32 TotalViewModes = ViewModes.Num();
	const float LastTs = Episode.GetLastTimestamp();
	const int32 CurrImgNr = Episode.GetCurrIndex() * TotalCameras * TotalViewModes + CurrVirtualCameraIdx * TotalViewModes + CurrViewModeNr;
	const int32 TotalImgs = TotalFrames * TotalCameras * TotalViewModes;

	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Camera=%ld/%ld; \t\t ViewMode=%ld/%ld; \t\t Image=%ld/%ld; \t\t Ts=%.2f/%.2f; \t\t Frame=%ld/%ld;"),
		*FString(__func__), __LINE__,		
		CurrCameraNr, TotalCameras,
		CurrViewModeNr, TotalViewModes,
		CurrImgNr, TotalImgs,
		CurrTimestamp, LastTs,
		CurrFrameNr, TotalFrames);
}

// Get view mode as string
FString USLVisionLogger::GetViewModeName(ESLVisionViewMode Mode) const
{
	if (Mode == ESLVisionViewMode::Color)
	{
		return FString("Color");
	}
	else if (Mode == ESLVisionViewMode::Unlit)
	{
		return FString("Unlit");
	}
	else if (Mode == ESLVisionViewMode::Mask)
	{
		return FString("Mask");
	}
	else if (Mode == ESLVisionViewMode::Depth)
	{
		return FString("Depth");
	}
	else if (Mode == ESLVisionViewMode::Normal)
	{
		return FString("Normal");
	}
	else
	{
		return FString("Other");
	}
}

