// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVScanner.h"
#include "CV/SLCVQScene.h"
#include "CV/SLCVUtils.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "FileHelper.h"

#include "Engine.h"
#include "Engine/PostProcessVolume.h"

#if WITH_EDITOR
#include "Engine/Selection.h"
#include "Editor.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

#if SL_WITH_DEBUG
#include "DrawDebugHelpers.h"
#endif // SL_WITH_DEBUG

// Sets default values
ASLCVScanner::ASLCVScanner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIgnore = true;
	bManualTrigger = false;
	bSaveToFile = false;
	bOverwrite = false;
	bPrintProgress = false;
	bUseIdsForFolderNames = false;
	bScanOnlySelectedIndividuals = true;
	bReplaceBackgroundPixels = false;
	bUseIndividualMaskValue = false;
	bDisablePostProcessVolumes = false;
	bDisableAO = false;

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Render all mdoes by default
	RenderModes.Add(ESLCVRenderMode::Lit);
	RenderModes.Add(ESLCVRenderMode::Unlit);
	RenderModes.Add(ESLCVRenderMode::Mask);
	RenderModes.Add(ESLCVRenderMode::Depth);
	RenderModes.Add(ESLCVRenderMode::Normal);

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLCVScanner"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Dtor
ASLCVScanner::~ASLCVScanner()
{
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		Finish(true);
	}
}

// Called when the game starts or when spawned
void ASLCVScanner::BeginPlay()
{
	Super::BeginPlay();

	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's ignore flag is true, skipping"), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	Init();

	//Start();
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ASLCVScanner::Start, 0.15f, false);
}

// Called when actor removed from game or game ended
void ASLCVScanner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish();
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLCVScanner::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLCVScanner, bAddSelectedIdsButton))
	{
		bAddSelectedIdsButton = false;
		if (bOverwriteSelectedIds)
		{
			SelectedIndividualsId.Empty();
			IdCSVString.ParseIntoArray(SelectedIndividualsId, TEXT(","));
		}
		else
		{
			TArray<FString> NewIds;
			IdCSVString.ParseIntoArray(NewIds, TEXT(","));
			for (const auto& Id : NewIds)
			{
				SelectedIndividualsId.AddUnique(Id);

			}			
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLCVScanner, bRemoveSelectedIdsButton))
	{
		bRemoveSelectedIdsButton = false;
		TArray<FString> NewIds;
		IdCSVString.ParseIntoArray(NewIds, TEXT(","));
		for (const auto& Id : NewIds)
		{
			SelectedIndividualsId.Remove(Id);
		}
	}
}
#endif // WITH_EDITOR

// Set up any required references and connect to server
void ASLCVScanner::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already initialized.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	FString ScanDir = FPaths::ProjectDir() + "/SL/" + TaskId + "/Scans/";
	FPaths::RemoveDuplicateSlashes(ScanDir);
	if (FPaths::DirectoryExists(ScanDir))
	{
		if (bOverwrite)
		{
			IFileManager::Get().DeleteDirectory(*ScanDir, false, true);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s scan directory %s already exists, deleting.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *ScanDir);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s scan directory %s already exists, mv or rm first.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *ScanDir);
			return;
		}
	}

	// Disable physiscs and detach all actors
	DetachAllActors();

	/* Set the individual manager */
	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!IndividualManager->IsLoaded() && !IndividualManager->Load(true))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		return;
	}

	/* Set the individuals to be scanned */
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (!SetScanIndividuals())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not find any visible individuals in the world (%s).."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *GetWorld()->GetName());
			return;
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		if (!SetScanScenes())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set any scan scenes.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}
	}

	// If no view modes are available, add a default one
	if (RenderModes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s no view modes found, added default one (lit).."),
			*FString(__func__), __LINE__, *GetName());
		RenderModes.Add(ESLCVRenderMode::Lit);
	}

	// Setup actor mask clones
	if (RenderModes.Contains(ESLCVRenderMode::Mask))
	{
		if (!SetMaskClones())
		{
			RenderModes.Remove(ESLCVRenderMode::Mask);
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s Could not setup mask clones .."),
				*FString(__func__), __LINE__, *GetName());
		}
	}

	// Set camera sphere poses
	if (!SetScanPoses(MaxNumScanPoints))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not setup camera scan points .."),
			*FString(__func__), __LINE__, *GetName());
	}

	/* Set the camera pose dummy actor */
	if (!SetCameraPoseAndLightActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the camera pose and light actor .."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	CameraPoseAndLightActor->SetActorTransform(FTransform::Identity);

	// Set the background mesh and material
	if (CustomBackgroundColor != FColor::Black && !bReplaceBackgroundPixels)
	{
		if (!SetBackgroundStaticMeshActor())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the bacground mesh actor .."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}
	}

	// Set ppv proerties (disable / ambient occlusion)
	SetPostProcessVolumeProperties();

	/* Set render and screenshot params */
	SetScreenshotResolution(Resolution);
	SetRenderParams();

	// Set and bind the viewport client screenshot callback
	ViewportClient = GetWorld()->GetGameViewport();
	if (!ViewportClient)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not access the GameViewport.."),
			*FString(__func__), __LINE__, *GetName());
		return;
	}

	// Bind screenshot callback
	ViewportClient->OnScreenshotCaptured().AddUObject(this, &ASLCVScanner::ScreenshotCapturedCallback);

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Start processing any incomming messages
void ASLCVScanner::Start()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already started.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, cannot start.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Make sure the player controller can be accessed
	if (!GetWorld()->GetFirstPlayerController())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s can only be started after begin play (PlayerController==nullptr) .."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Clear physics on all actors
	DisablePhysicsOnAllActors();

	// Remove hide all actors in the world
	HideAllActors();

	// Make sure the camera light is not hidden
	CameraPoseAndLightActor->SetActorHiddenInGame(false);
	CameraPoseAndLightActor->GetLightComponent()->SetVisibility(true);

	// Make sure pawn is not in the scene
	GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);

	// Set the first individual
	IndividualOrSceneIdx = INDEX_NONE;
	
	if (!SetNextScene())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set up first scene, aborting scan .."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (bManualTrigger)
	{
		// Bind user inputs
		SetupInputBindings();
	}
	else
	{		
		// Start the dominoes
		//RequestScreenshotAsync();

		// Use a delay, sometimes the materials are not properly loaded
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ASLCVScanner::RequestScreenshotAsync, 0.15f, false);
	}

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Stop processing the messages, and disconnect from server
void ASLCVScanner::Finish(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already finished.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit && !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized nor started, cannot finish.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully finished.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Setup user input bindings
void ASLCVScanner::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(UserInputActionName, IE_Pressed, this, &ASLCVScanner::RequestScreenshotAsync);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access player controller (make sure it is not called before BeginPlay).."), *FString(__FUNCTION__), __LINE__);
	}
}

// Request a high res screenshot
void ASLCVScanner::RequestScreenshotAsync()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
		{
			GetHighResScreenshotConfig().FilenameOverride = CurrImageName;
			ViewportClient->Viewport->TakeHighResScreenShot();
		});
}

// Called when the screenshot is captured
void ASLCVScanner::ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap)
{
	// Print to terminal the progress state
	if (bPrintProgress)
	{
		PrintProgress();
	}

	// Compressed image
	TArray<uint8> CompressedBitmap;

	// Check if the background should be replaced or not
	if (bReplaceBackgroundPixels)
	{
		// Switch pixel colors (switch black background color with a custom one)
		TArray<FColor> NewImage  = FSLCVUtils::ReplacePixels(InBitmap, FColor::Black, CustomBackgroundColor, CustomBackgroundColorTolerance);

		// Compress the modified image
		FImageUtils::CompressImageArray(SizeX, SizeY, NewImage, CompressedBitmap);
	}
	else
	{
		// Compress the original image
		FImageUtils::CompressImageArray(SizeX, SizeY, InBitmap, CompressedBitmap);
	}

	// Check if the image should be stored locally
	if (bSaveToFile)
	{
		SaveToFile(CompressedBitmap);
	}

	// Set and trigger the next shot
	if (SetNextRenderMode())
	{
		if (!bManualTrigger)
		{
			RequestScreenshotAsync();
		}
	}
	else
	{
		if (SetNextCameraPose())
		{
			if (!bManualTrigger)
			{
				RequestScreenshotAsync();
			}
		}
		else
		{
			if (SetNextScene())
			{
				if (!bManualTrigger)
				{
					//RequestScreenshotAsync();

					// Use a delay, sometimes the materials are not properly loaded
					FTimerHandle UnusedHandle;
					GetWorldTimerManager().SetTimer(UnusedHandle, this, &ASLCVScanner::RequestScreenshotAsync, 0.15f, false);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s finished, quitting editor.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName());
				Finish();
				
				// todo, try to get the camera to the starting pose
				GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(false);
				GetWorld()->GetFirstPlayerController()->SetViewTarget(GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator());

				QuitEditor();
			}
		}
	}
}

// Set next view mode (return false if the last view mode was reached)
bool ASLCVScanner::SetNextRenderMode()
{
	RenderModeIdx++;
	if (RenderModes.IsValidIndex(RenderModeIdx))
	{
		// Apply desired view mode
		ApplyRenderMode(RenderModes[RenderModeIdx]);

		// Set image name
		SetImageName();
		return true;
	}
	else
	{
		RenderModeIdx = INDEX_NONE;
		return false;
	}
}

// Set next camera pose (return false if the last pose was reached)
bool ASLCVScanner::SetNextCameraPose()
{
	CameraPoseIdx++;
	if (CameraScanUnitPoses.IsValidIndex(CameraPoseIdx))
	{
		// Goto first view mode
		RenderModeIdx = INDEX_NONE;
		SetNextRenderMode();

		// Move camera to the desired pose
		ApplyCameraPose(CameraScanUnitPoses[CameraPoseIdx]);

		// Set image name
		CameraPoseIdxString = FString::FromInt(CameraPoseIdx) + "_" + FString::FromInt(CameraScanUnitPoses.Num());
		SetImageName();

		return true;
	}
	else
	{
		CameraPoseIdx = INDEX_NONE;
		return false;
	}
}

// Set next view mode (return false if the last view mode was reached)
bool ASLCVScanner::SetNextScene()
{
	IndividualOrSceneIdx++;
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (Individuals.IsValidIndex(IndividualOrSceneIdx))
		{
			// Move the individual into position
			ApplyIndividual(Individuals[IndividualOrSceneIdx]);

			// Update camera distance from individual
			SetCameraPoseSphereRadius();

			// Goto first camera pose
			CameraPoseIdx = INDEX_NONE;
			SetNextCameraPose();

			// Set individual string
			SceneNameString = bUseIdsForFolderNames ? Individuals[IndividualOrSceneIdx]->GetIdValue()
				//: Individuals[IndividualOrSceneIdx]->GetParentActor()->GetName();
				: Individuals[IndividualOrSceneIdx]->GetClassValue();

			// Set image name
			IndividualOrSceneIdxString = FString::FromInt(IndividualOrSceneIdx) + "_" + FString::FromInt(Individuals.Num());
			SetImageName();

			return true;
		}
		else
		{
			IndividualOrSceneIdx = INDEX_NONE;
			return false;
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		if (Scenes.IsValidIndex(IndividualOrSceneIdx))
		{
			// Set the scene individuals in position
			ApplyScene();

			// Update camera distance from individual
			SetCameraPoseSphereRadius();

			// Goto first camera pose
			CameraPoseIdx = INDEX_NONE;
			SetNextCameraPose();

			// Set individual string
			SceneNameString = Scenes[IndividualOrSceneIdx]->GetSceneName();

			// Set image name
			IndividualOrSceneIdxString = FString::FromInt(IndividualOrSceneIdx) + "_" + FString::FromInt(Scenes.Num());
			SetImageName();

			return true;
		}
		else
		{
			IndividualOrSceneIdx = INDEX_NONE;
			return false;
		}
	}
	else
	{
		return false;
	}
}

// Quit the editor once the scanning is finished
void ASLCVScanner::QuitEditor()
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

// Apply the selected view mode
void ASLCVScanner::ApplyRenderMode(ESLCVRenderMode NewRenderMode)
{
	// No change in the rendering view mode
	if (NewRenderMode == PrevRenderMode)
	{
		return;
	}

	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if (NewRenderMode == ESLCVRenderMode::Lit)
	{
		RenderModeString = "L";
		if (PrevRenderMode == ESLCVRenderMode::Depth || PrevRenderMode == ESLCVRenderMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else
		{
			if (PrevRenderMode == ESLCVRenderMode::Mask)
			{
				ShowOriginalIndividual();
			}
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}		
	}
	else if (NewRenderMode == ESLCVRenderMode::Unlit)
	{
		RenderModeString = "U";
		if (PrevRenderMode == ESLCVRenderMode::Mask)
		{
			ShowOriginalIndividual();
		}
		else
		{
			if (PrevRenderMode == ESLCVRenderMode::Depth || PrevRenderMode == ESLCVRenderMode::Normal)
			{
				ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			}
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
	}
	else if (NewRenderMode == ESLCVRenderMode::Mask)
	{
		RenderModeString = "M";
		if (PrevRenderMode != ESLCVRenderMode::Unlit)
		{
			if (PrevRenderMode == ESLCVRenderMode::Depth || PrevRenderMode == ESLCVRenderMode::Normal)
			{
				ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			}
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		ShowMaskIndividual();
	}
	else if (NewRenderMode == ESLCVRenderMode::Depth)
	{
		RenderModeString = "D";
		if (PrevRenderMode != ESLCVRenderMode::Normal)
		{
			if (PrevRenderMode != ESLCVRenderMode::Lit)
			{
				GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
				if (PrevRenderMode == ESLCVRenderMode::Mask)
				{
					ShowOriginalIndividual();
				}
			}
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		}
		// SLSceneDepthToCameraPlane / SLSceneDepthToCameraLocation / SLCVScanDepthToCameraPlane / SLCVScanDepthToCameraLocation 
		if (CurrCameraPoseSphereRadius < 100.f)
		{
			BufferVisTargetCV->Set(*FString("SLCVScanDepthToCameraPlaneMacro"));
		}
		else
		{
			BufferVisTargetCV->Set(*FString("SLCVScanDepthToCameraPlane"));
		}
	}
	else if (NewRenderMode == ESLCVRenderMode::Normal)
	{
		RenderModeString = "N";
		if (PrevRenderMode != ESLCVRenderMode::Depth)
		{
			if (PrevRenderMode != ESLCVRenderMode::Lit)
			{
				GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
				if (PrevRenderMode == ESLCVRenderMode::Mask)
				{
					ShowOriginalIndividual();
				}
			}
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
		}
		BufferVisTargetCV->Set(*FString("WorldNormal"));
	}	
	PrevRenderMode = NewRenderMode;
}

// Apply the camera pose
void ASLCVScanner::ApplyCameraPose(FTransform UnitSpherePose)
{
	UnitSpherePose.AddToTranslation(UnitSpherePose.GetTranslation() * CurrCameraPoseSphereRadius);
	CameraPoseAndLightActor->SetActorTransform(UnitSpherePose);
	GetWorld()->GetFirstPlayerController()->SetViewTarget(CameraPoseAndLightActor);
}

// Apply the individual into position
void ASLCVScanner::ApplyIndividual(USLVisibleIndividual* Individual)
{
	// Hide any previous individual
	const int32 PrevIdx = IndividualOrSceneIdx - 1;
	if (Individuals.IsValidIndex(PrevIdx))
	{
		auto PrevIndividual = Individuals[PrevIdx];
		PrevIndividual->GetParentActor()->SetActorHiddenInGame(true);
		// Hide mask
		if (auto Clone = IndividualsMaskClones.Find(PrevIndividual))
		{
			(*Clone)->SetActorHiddenInGame(true);
		}
	}

	// Set current individual (and clone) into position
	Individual->GetParentActor()->SetActorHiddenInGame(false);
	FTransform InitialPose = FTransform::Identity;
	Individual->GetParentActor()->SetActorTransform(InitialPose);
	if (auto Clone = IndividualsMaskClones.Find(Individual))
	{
		(*Clone)->SetActorTransform(InitialPose);
	}
}

// Apply the scene into position
void ASLCVScanner::ApplyScene()
{	
	// Hide previous scene
	int32 PrevSceneIdx = IndividualOrSceneIdx - 1;
	if (Scenes.IsValidIndex(PrevSceneIdx))
	{
		Scenes[PrevSceneIdx]->HideScene();
	}

	// Show current scene
	Scenes[IndividualOrSceneIdx]->ShowScene();
}

// Hide mask clone, show original individual
void ASLCVScanner::ShowOriginalIndividual()
{
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (Individuals.IsValidIndex(IndividualOrSceneIdx))
		{
			auto Individual = Individuals[IndividualOrSceneIdx];
			Individual->GetParentActor()->SetActorHiddenInGame(false);
			if (auto Clone = IndividualsMaskClones.Find(Individual))
			{
				(*Clone)->SetActorHiddenInGame(true);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual clone is not found, this should not happen .."),
					*FString(__FUNCTION__), __LINE__, *GetName(), IndividualOrSceneIdx, Individuals.Num());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual index is not valid %d/%d, this should not happen .."),
				*FString(__FUNCTION__), __LINE__, *GetName(), IndividualOrSceneIdx, Individuals.Num());
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		if (Scenes.IsValidIndex(IndividualOrSceneIdx))
		{
			Scenes[IndividualOrSceneIdx]->ShowOriginalMaterials();
		}		
	}
}

// Hide original individual, show mask clone
void ASLCVScanner::ShowMaskIndividual()
{
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (Individuals.IsValidIndex(IndividualOrSceneIdx))
		{
			auto Individual = Individuals[IndividualOrSceneIdx];
			Individual->GetParentActor()->SetActorHiddenInGame(true);
			if (auto Clone = IndividualsMaskClones.Find(Individual))
			{
				(*Clone)->SetActorHiddenInGame(false);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual clone is not found, this should not happen .."),
					*FString(__FUNCTION__), __LINE__, IndividualOrSceneIdx, Individuals.Num());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual index is not valid %d/%d, this should not happen .."),
				*FString(__FUNCTION__), __LINE__, IndividualOrSceneIdx, Individuals.Num());
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		if (Scenes.IsValidIndex(IndividualOrSceneIdx))
		{
			Scenes[IndividualOrSceneIdx]->ShowMaskMaterials();
		}
	}
}

// Remove detachments and hide all actors in the world
void ASLCVScanner::HideAllActors()
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{		
		// Hide by default
		ActItr->SetActorHiddenInGame(true);
	}
}

// Detach all actors
void ASLCVScanner::DetachAllActors()
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Clear any attachments between actors
		ActItr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

// Disable physiscs and detach all actors
void ASLCVScanner::DisablePhysicsOnAllActors()
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Make sure all actors have no physics, have no collisions and are movable
		ActItr->DisableComponentsSimulatePhysics();

		ActItr->SetActorEnableCollision(ECollisionEnabled::NoCollision);
		if (ActItr->GetRootComponent())
		{
			ActItr->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		}
	}
}

// Set ppv proerties (disable / ambient occlusion)
void ASLCVScanner::SetPostProcessVolumeProperties()
{
	if (bDisablePostProcessVolumes || bDisableAO)
	{
		for (TActorIterator<APostProcessVolume> PPVItr(GetWorld()); PPVItr; ++PPVItr)
		{
			if (bDisablePostProcessVolumes)
			{
				PPVItr->bEnabled = false;
				PPVItr->BlendWeight = 0.f;
			}
			else if (bDisableAO)
			{
				PPVItr->Settings.AmbientOcclusionIntensity = 0.f;
			}
		}
	}
}

// Set screenshot image resolution
void ASLCVScanner::SetScreenshotResolution(FIntPoint InResolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(InResolution.X, InResolution.Y, 1.0f);
	// !! Workaround !! Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;
}

// Set the rendering parameters
void ASLCVScanner::SetRenderParams()
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
	if (GEngine)
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

	if (bDisableAO)
	{
		// 0: off Engine default (project setting) for AmbientOcclusion is (postprocess volume/camera/game setting still can override)
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AmbientOcclusion"))->Set(0);
	}
}

// Get the individual manager from the world (or spawn a new one)
bool ASLCVScanner::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable())
	{
		return true;
	}
	if (GetWorld())
	{
		IndividualManager = ASLIndividualManager::GetExistingOrSpawnNew(GetWorld());
	}
	return IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable();
}

// Get the mongo query manager (used to set up scenes from episodic memories)
bool ASLCVScanner::SetMongoQueryManager()
{
	if (MongoQueryManager && MongoQueryManager->IsValidLowLevel() && !MongoQueryManager->IsPendingKillOrUnreachable())
	{
		return true;
	}
	if (GetWorld())
	{
		MongoQueryManager = ASLMongoQueryManager::GetExistingOrSpawnNew(GetWorld());
	}
	return MongoQueryManager && MongoQueryManager->IsValidLowLevel() && !MongoQueryManager->IsPendingKillOrUnreachable();
}

// Set the individuals to be scanned
bool ASLCVScanner::SetScanIndividuals()
{
	if (bScanOnlySelectedIndividuals)
	{
		for (const auto& Id : SelectedIndividualsId)
		{
			if (auto BI = IndividualManager->GetIndividual(Id))
			{
				if (auto AsVI = Cast<USLVisibleIndividual>(BI))
				{
					Individuals.Add(AsVI);
				}
			}			
		}
	}
	else
	{
		for (const auto& BI : IndividualManager->GetIndividuals())
		{
			if (auto AsVI = Cast<USLVisibleIndividual>(BI))
			{
				if (auto AsSMA = Cast<AStaticMeshActor>(BI->GetParentActor()))
				{
					if (AsSMA->GetStaticMeshComponent()->Bounds.GetSphere().W < MaxBoundsSphereRadius)
					{
						Individuals.Add(AsVI);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d %s' %s is too large to be scanned %f/%f .."),
							*FString(__FUNCTION__), __LINE__, *GetName(),
							*AsSMA->GetName(), AsSMA->GetStaticMeshComponent()->Bounds.GetSphere().W, MaxBoundsSphereRadius);
					}
				}
			}
		}
	}
	return Individuals.Num() > 0;
}

// Set the scenes to be scanned
bool ASLCVScanner::SetScanScenes()
{
	if (Scenes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s no scenes found, aborting scan .."),
			*FString(__func__), __LINE__, *GetName());
		return false;
	}

	if (!SetMongoQueryManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the mongo query manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	TArray<USLCVQScene*> ScenesToRemove;
	for (auto& Scene : Scenes)
	{
		// Remove scene if it is flagged with ignore
		if (Scene->bIgnore)
		{
			ScenesToRemove.Add(Scene);
			continue;
		}

		// Query and cache the scene actors poses
		if (!Scene->InitScene(IndividualManager, MongoQueryManager))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not init scene %s.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *Scene->GetName());
			ScenesToRemove.Add(Scene);
		}
	}

	// Remove scenes
	for (auto* Scene : ScenesToRemove)
	{
		Scenes.Remove(Scene);
	}

	return Scenes.Num() > 0;
}

// Spawn a light actor which will also be used to move the camera around
bool ASLCVScanner::SetCameraPoseAndLightActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_CameraLightAndPose");
	CameraPoseAndLightActor = GetWorld()->SpawnActor<ADirectionalLight>(SpawnParams);
#if WITH_EDITOR
	CameraPoseAndLightActor->SetActorLabel(SpawnParams.Name.ToString());
#endif // WITH_EDITOR
	CameraPoseAndLightActor->SetMobility(EComponentMobility::Movable);
	CameraPoseAndLightActor->GetLightComponent()->SetIntensity(CameraLightIntensity);
	return true;
}

// Create clones of the individuals with mask material
bool ASLCVScanner::SetMaskClones()
{
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		GenerateMaskClones(Individuals);
		return IndividualsMaskClones.Num() > 0;
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		for (const auto& Scene : Scenes)
		{
			if (bUseIndividualMaskValue)
			{
				Scene->GenerateMaskClones(DynMaskMatAssetPath);
			}
			else
			{
				Scene->GenerateMaskClones(DynMaskMatAssetPath, false, MaskColor);
			}
		}
		// todo
		return true;
	}
	return false;
}

// Generate mask clones from the ids
void ASLCVScanner::GenerateMaskClones(const TArray<USLVisibleIndividual*>& VisibleIndividuals)
{
	// Get the dynamic mask material
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this, DynMaskMatAssetPath);
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load default mask material.."),
			*FString(__func__), __LINE__, *GetName());
		return;
	}
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Create a commong dynamic mask material and set its color
	UMaterialInstanceDynamic* DynamicMaskMaterial = nullptr;
	if (!bUseIndividualMaskValue)
	{
		DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), MaskColor);
	}

	for (const auto& VI : VisibleIndividuals)
	{
		// Avoid creating duplicates
		if (IndividualsMaskClones.Contains(VI))
		{
			continue;
		}

		// Use the individual unique visual mask value for the mask
		if (bUseIndividualMaskValue)
		{
			DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
			DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FColor::FromHex(VI->GetVisualMaskValue()));
		}

		// Make sure parent is a static mesh actor
		if (auto AsSMA = Cast<AStaticMeshActor>(VI->GetParentActor()))
		{
			FActorSpawnParameters Parameters;
			Parameters.Template = AsSMA;
			Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			//Parameters.Instigator = SMA->GetInstigator();
			Parameters.Name = FName(*(AsSMA->GetName() + TEXT("_MaskClone")));
			AStaticMeshActor* SMAClone = GetWorld()->SpawnActor<AStaticMeshActor>(AsSMA->GetClass(), Parameters);
#if WITH_EDITOR
			SMAClone->SetActorLabel(Parameters.Name.ToString());
#endif // WITH_EDITOR
			if (UStaticMeshComponent* SMC = SMAClone->GetStaticMeshComponent())
			{
				for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
				{
					SMC->SetMaterial(MatIdx, DynamicMaskMaterial);
				}
			}
			SMAClone->DisableComponentsSimulatePhysics();
			SMAClone->SetActorHiddenInGame(true);
			IndividualsMaskClones.Add(VI, SMAClone);
		}
		else if (auto AsSkelMA = Cast<ASkeletalMeshActor>(VI->GetParentActor()))
		{
			//todo
		}
	}
}

// Set the background static mesh actor and material
bool ASLCVScanner::SetBackgroundStaticMeshActor()
{
	// Spawn actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_BackgroundSphereMesh");
	BackgroundSMA = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
#if WITH_EDITOR
	BackgroundSMA->SetActorLabel(SpawnParams.Name.ToString());
#endif // WITH_EDITOR

	// Set the mesh component
	UStaticMesh* BackgroundSM = LoadObject<UStaticMesh>(nullptr, BackgroundAssetPath);
	if (!BackgroundSM)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load background mesh.."), *FString(__func__), __LINE__);
		BackgroundSMA->Destroy();
		return false;
	}
	BackgroundSMA->GetStaticMeshComponent()->SetStaticMesh(BackgroundSM);
	BackgroundSMA->SetMobility(EComponentMobility::Movable);
	FTransform T(FRotator::ZeroRotator, FVector::ZeroVector, FVector(400));
	BackgroundSMA->SetActorTransform(T);

	UMaterial* BackgroundMaterial = LoadObject<UMaterial>(this, BackgroundDynMatAssetPath);
	if (!BackgroundMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load background material.."), *FString(__func__), __LINE__);
		BackgroundSMA->Destroy();
		return false;
	}
	//DefaultMaskMaterial->bUsedWithStaticLighting = true;
	//DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BackgroundMaterial, GetTransientPackage());
	MID->SetVectorParameterValue(FName("MaskColorParam"), CustomBackgroundColor);
#if WITH_EDITOR	
	BackgroundSM->SetMaterial(0, MID);
#endif
	return true;
}

// Generate sphere camera scan poses
bool ASLCVScanner::SetScanPoses(uint32 MaxNumPoints/*, float Radius*/)
{
	// (https://www.cmu.edu/biolphys/deserno/pdf/sphere_equi.pdf)
	const float Area = 4 * PI / MaxNumPoints;
	const float Distance = FMath::Sqrt(Area);

	// Num of latitudes
	const int32 MTheta = FMath::RoundToInt(PI / Distance);
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
			Point.X = FMath::Sin(Theta) * FMath::Cos(Phi)/* * Radius*/;
			Point.Y = FMath::Sin(Theta) * FMath::Sin(Phi)/* * Radius*/;
			Point.Z = FMath::Cos(Theta)/* * Radius*/;
			FQuat Quat = (-Point).ToOrientationQuat();

			FTransform UnitSpherePose = FTransform(Quat, Point);
			CameraScanUnitPoses.Emplace(UnitSpherePose);
		}
	}
	return CameraScanUnitPoses.Num() > 0;
}

// Set the image name
void ASLCVScanner::SetImageName()
{
	//CurrImageName = ViewIdxString + "_" + CameraPoseIdxString + "_" + ViewModeString;
	//CurrImageName = RenderModeString + "_" + IndividualOrSceneIdxString + "_" + CameraPoseIdxString;
	//CurrImageName = RenderModeString + "_" + FString::FromInt(CameraPoseIdx);
	
	CurrImageName = RenderModeString + "/img" + FString::FromInt(10000 + CameraPoseIdx); //ffmpg friendly
	
	//int32 CurrIdx = CameraPoseIdx * RenderModes.Num() + RenderModeIdx + 1;
	//CurrImageName = "/img" + FString::FromInt(10000 + CurrIdx); //ffmpg friendly
}

// Calculate camera pose sphere radius (proportionate to the sphere bounds of the visual mesh)
void ASLCVScanner::SetCameraPoseSphereRadius()
{
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (Individuals.IsValidIndex(IndividualOrSceneIdx))
		{
			if (auto AsSMA = Cast<AStaticMeshActor>(Individuals[IndividualOrSceneIdx]->GetParentActor()))
			{
				const float SphereRadius = AsSMA->GetStaticMeshComponent()->Bounds.SphereRadius;
				CurrCameraPoseSphereRadius = SphereRadius * CameraRadiusDistanceMultiplier;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual %s is not of a supported type.."),
					*FString(__FUNCTION__), __LINE__, *GetName(), *Individuals[IndividualOrSceneIdx]->GetParentActor()->GetName());

				FVector BBOrigin;
				FVector BBBoxExtent;
				Individuals[IndividualOrSceneIdx]->GetParentActor()->GetActorBounds(false, BBOrigin, BBBoxExtent);
				CurrCameraPoseSphereRadius = BBBoxExtent.Size() * CameraRadiusDistanceMultiplier;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual index is not valid %d/%d, this should not happen .."),
				*FString(__FUNCTION__), __LINE__, *GetName(), IndividualOrSceneIdx, Individuals.Num());
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		//const float SphereRadius = Scenes[IndividualOrSceneIdx]->GetSphereBoundsRadius();
		const float SphereRadius = Scenes[IndividualOrSceneIdx]->GetAppliedSceneSphereBoundsRadius();
		CurrCameraPoseSphereRadius = SphereRadius * CameraRadiusDistanceMultiplier;
	}

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
	// Used to calculate the dynamic arrow color
	int32 PoseIdx = 0;
	// Draw the camera poses	
	for (const auto& CameraUnitPose : CameraScanUnitPoses)
	{
		// Arrow locations
		FVector SphereCenter = FVector::ZeroVector;
		FVector ArrowOrigin = CameraUnitPose.GetLocation() * CurrCameraPoseSphereRadius;
		FVector ArrowEnd = (ArrowOrigin + SphereCenter) * ArrowHeadLocPerc;
		FColor ArrowColor = FColor::Green;

		// Scalar to map the arrow color (0 .. 1)
		float ColorScalar = (1.f / CameraScanUnitPoses.Num()) * PoseIdx;

		// 0.f - Red; 1.f - Green;
		//ArrowColor = FColor::MakeRedToGreenColorFromScalar(ColorScalar);		

		// 1000K - Red; 15000K - White;
		//ColorScalar += (ColorScalar * 14000.f) + 1000.f;
		//ArrowColor = FColor::MakeFromColorTemperature(ColorScalar); 

		// Lerp between colors
		ArrowColor = FLinearColor::LerpUsingHSV(FLinearColor(StartColorLerp), FLinearColor(EndColorLerp), ColorScalar).ToFColor(true);

		// Draw the debug arrow
		const float LifeTime = -1.f;
		const uint8 DepthPriority = 0;
		DrawDebugDirectionalArrow(GetWorld(), ArrowOrigin, ArrowEnd, DebugArrowHeadSize, ArrowColor, true,
			LifeTime, DepthPriority, DebugArrowThickness);

		PoseIdx++;
	}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
}

// Print progress to terminal
void ASLCVScanner::PrintProgress() const
{
	// Current scan
	int32 CurrScan = IndividualOrSceneIdx * CameraScanUnitPoses.Num() * RenderModes.Num() 
		+ CameraPoseIdx * RenderModes.Num() 
		+ RenderModeIdx + 1;

	int32 TotalScenesOrIndividuals = ScanMode == ESLCVScanMode::Individuals ? Individuals.Num() : Scenes.Num();
	int32 TotalNumScans = TotalScenesOrIndividuals * CameraScanUnitPoses.Num() * RenderModes.Num();

	UE_LOG(LogTemp, Log, TEXT("%s::%d::%f Individual/Scene:\t%ld/%ld; CameraPose:\t%ld/%ld; ViewMode:\t%ld/%ld; Scan:\t%ld/%ld;"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
		IndividualOrSceneIdx + 1, TotalScenesOrIndividuals,
		CameraPoseIdx + 1, CameraScanUnitPoses.Num(),
		RenderModeIdx + 1, RenderModes.Num(),
		CurrScan, TotalNumScans);
}

// Save image to file
void ASLCVScanner::SaveToFile(const TArray<uint8>& CompressedBitmap) const
{
	//const FString TaskFolderPath = TaskId + "/Scans/" + IndividualId + "/" + ViewModeString + "/";
	const FString TaskFolderPath = "/SL/" + TaskId + "/Scans/" + SceneNameString + /*"/" + ViewModeString*/ + "/";
	FString Path = FPaths::ProjectDir() + TaskFolderPath + CurrImageName + ".png";
	FPaths::RemoveDuplicateSlashes(Path);
	FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);

	// Include image in a folder with all of them mixed
	int32 CurrMixedIdx = CameraPoseIdx * RenderModes.Num() + RenderModeIdx + 1;
	const FString CurrMixedImageName = "A/img" + FString::FromInt(10000 + CurrMixedIdx); //ffmpg friendly

	FString MixedPath = FPaths::ProjectDir() + TaskFolderPath + CurrMixedImageName + ".png";
	FPaths::RemoveDuplicateSlashes(MixedPath);
	FFileHelper::SaveArrayToFile(CompressedBitmap, *MixedPath);
}
