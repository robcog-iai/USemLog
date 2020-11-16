// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVScanner.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLVisibleIndividual.h"
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

// Sets default values
ASLCVScanner::ASLCVScanner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIgnore = true;
	bSaveToFile = false;
	bPrintProgress = false;
	bUseActorNamesForFolders = true;
	bScanOnlySelectedIndividuals = true;
	bUseIndividualMaskValue = false;
	bDisablePostProcessVolumes = false;
	bDisableAO = false;

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Render all mdoes by default
	ViewModes.Add(ESLCVViewMode::Lit);
	ViewModes.Add(ESLCVViewMode::Unlit);
	ViewModes.Add(ESLCVViewMode::Mask);
	ViewModes.Add(ESLCVViewMode::Depth);
	ViewModes.Add(ESLCVViewMode::Normal);

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
	Start();
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s scan directory %s already exists, mv or rm first.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *ScanDir);
		return;
	}

	// Remove detachments and hide all actors in the world
	SetWorldState();

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
		if (Scenes.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s no scenes found, aborting scan .."),
				*FString(__func__), __LINE__, *GetName());
		}
	}

	// If no view modes are available, add a default one
	if (ViewModes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s no view modes found, added default one (lit).."),
			*FString(__func__), __LINE__, *GetName());
		ViewModes.Add(ESLCVViewMode::Lit);
	}

	// Setup actor mask clones
	if (ViewModes.Contains(ESLCVViewMode::Mask))
	{
		if (!SetMaskClones())
		{
			ViewModes.Remove(ESLCVViewMode::Mask);
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
	CameraPoseAndLightActor->SetActorTransform(FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f)));

	// Set the background mesh and material
	if (BackgroundColor != FColor::Black)
	{
		if (!SetBackgroundStaticMeshActor())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the bacground mesh actor .."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}
	}

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

	// Make sure pawn is not in the scene
	GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);

	// Set the first individual
	ViewIdx = INDEX_NONE;
	SetNextView();

	// Start the dominoes
	RequestScreenshotAsync();

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

	// Check if the image should be stored locally
	if (bSaveToFile)
	{
		SaveToFile(SizeX, SizeY, InBitmap);
	}

	// Set and trigger the next shot
	if (SetNextViewMode())
	{
		RequestScreenshotAsync();
	}
	else
	{
		if (SetNextCameraPose())
		{
			RequestScreenshotAsync();
		}
		else
		{
			if (SetNextView())
			{
				RequestScreenshotAsync();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s finished, quitting editor.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName());
				Finish();
				QuitEditor();
			}
		}
	}
}

// Set next view mode (return false if the last view mode was reached)
bool ASLCVScanner::SetNextViewMode()
{
	ViewModeIdx++;
	if (ViewModes.IsValidIndex(ViewModeIdx))
	{
		// Apply desired view mode
		ApplyViewMode(ViewModes[ViewModeIdx]);

		// Set image name
		SetImageName();
		return true;
	}
	else
	{
		ViewModeIdx = INDEX_NONE;
		return false;
	}
}

// Set next camera pose (return false if the last pose was reached)
bool ASLCVScanner::SetNextCameraPose()
{
	CameraPoseIdx++;
	if (CameraScanPoses.IsValidIndex(CameraPoseIdx))
	{
		// Goto first view mode
		ViewModeIdx = INDEX_NONE;
		SetNextViewMode();

		// Move camera to the desired pose
		ApplyCameraPose(CameraScanPoses[CameraPoseIdx]);

		// Set image name
		CameraPoseIdxString = FString::FromInt(CameraPoseIdx) + "_" + FString::FromInt(CameraScanPoses.Num());
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
bool ASLCVScanner::SetNextView()
{
	ViewIdx++;
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (Individuals.IsValidIndex(ViewIdx))
		{
			// Update camera distance from individual
			CalcCameraPoseSphereRadius();

			// Goto first camera pose
			CameraPoseIdx = INDEX_NONE;
			SetNextCameraPose();

			// Move the individual into position
			ApplyIndividual(Individuals[ViewIdx]);

			// Set individual string
			ViewNameString = bUseActorNamesForFolders ? Individuals[ViewIdx]->GetParentActor()->GetName() 
				: Individuals[ViewIdx]->GetIdValue();

			// Set image name
			ViewIdxString = FString::FromInt(ViewIdx) + "_" + FString::FromInt(Individuals.Num());
			SetImageName();

			return true;
		}
		else
		{
			ViewIdx = INDEX_NONE;
			return false;
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		if (Scenes.IsValidIndex(ViewIdx))
		{
			// Update camera distance from individual
			CalcCameraPoseSphereRadius();

			// Goto first camera pose
			CameraPoseIdx = INDEX_NONE;
			SetNextCameraPose();

			// Set the scene individuals in position
			ApplyScene();

			// Set individual string
			ViewNameString = Scenes[ViewIdx];

			// Set image name
			ViewIdxString = FString::FromInt(ViewIdx) + "_" + FString::FromInt(Scenes.Num());
			SetImageName();

			return true;
		}
		else
		{
			ViewIdx = INDEX_NONE;
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
void ASLCVScanner::ApplyViewMode(ESLCVViewMode NewViewMode)
{
	// No change in the rendering view mode
	if (NewViewMode == CurrViewmode)
	{
		return;
	}

	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if (NewViewMode == ESLCVViewMode::Lit)
	{
		if (CurrViewmode == ESLCVViewMode::Depth || CurrViewmode == ESLCVViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else if (CurrViewmode == ESLCVViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else if (CurrViewmode == ESLCVViewMode::Mask)
		{
			ShowOriginalIndividual();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		ViewModeString = "L";
	}
	else if (NewViewMode == ESLCVViewMode::Unlit)
	{
		if (CurrViewmode == ESLCVViewMode::Lit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if (CurrViewmode == ESLCVViewMode::Mask)
		{
			ShowOriginalIndividual();
		}
		else if (CurrViewmode == ESLCVViewMode::Depth || CurrViewmode == ESLCVViewMode::Normal)
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
	else if (NewViewMode == ESLCVViewMode::Mask)
	{
		if (CurrViewmode == ESLCVViewMode::Unlit)
		{
			ShowMaskIndividual();
		}
		else if (CurrViewmode == ESLCVViewMode::Lit)
		{
			ShowMaskIndividual();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if (CurrViewmode == ESLCVViewMode::Depth || CurrViewmode == ESLCVViewMode::Normal)
		{
			ShowMaskIndividual();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ShowMaskIndividual();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		ViewModeString = "M";
	}
	else if (NewViewMode == ESLCVViewMode::Depth)
	{
		if (CurrViewmode == ESLCVViewMode::Mask)
		{
			ShowOriginalIndividual();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			//BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraLocation"));
		}
		else if (CurrViewmode == ESLCVViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			//BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraLocation"));
		}
		else if (CurrViewmode == ESLCVViewMode::Lit)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			//BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraLocation"));
		}
		else if (CurrViewmode == ESLCVViewMode::Normal)
		{
			//BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraLocation"));
		}
		else
		{
			ShowOriginalIndividual();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			//BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraLocation"));
		}
		ViewModeString = "D";
	}
	else if (NewViewMode == ESLCVViewMode::Normal)
	{
		if (CurrViewmode == ESLCVViewMode::Depth)
		{
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if (CurrViewmode == ESLCVViewMode::Mask)
		{
			ShowOriginalIndividual();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if (CurrViewmode == ESLCVViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if (CurrViewmode == ESLCVViewMode::Lit)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else
		{
			ShowOriginalIndividual();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		ViewModeString = "N";
	}
	CurrViewmode = NewViewMode;
}

// Apply the camera pose
void ASLCVScanner::ApplyCameraPose(FTransform NormalizedTransform)
{
	NormalizedTransform.SetTranslation(NormalizedTransform.GetTranslation() * CurrCameraPoseSphereRadius);
	CameraPoseAndLightActor->SetActorTransform(NormalizedTransform);
	GetWorld()->GetFirstPlayerController()->SetViewTarget(CameraPoseAndLightActor);
}

// Apply the individual into position
void ASLCVScanner::ApplyIndividual(USLVisibleIndividual* Individual)
{
	// Hide any previous individual
	const int32 PrevIdx = ViewIdx - 1;
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
	UE_LOG(LogTemp, Error, TEXT("%s::%d Applying scene %s .. "),
		*FString(__FUNCTION__), __LINE__, *Scenes[ViewIdx]);
}

// Hide mask clone, show original individual
void ASLCVScanner::ShowOriginalIndividual()
{
	if (Individuals.IsValidIndex(ViewIdx))
	{
		auto Individual = Individuals[ViewIdx];
		Individual->GetParentActor()->SetActorHiddenInGame(false);
		if (auto Clone = IndividualsMaskClones.Find(Individual))
		{
			(*Clone)->SetActorHiddenInGame(true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual clone is not found, this should not happen .."),
				*FString(__FUNCTION__), __LINE__, *GetName(), ViewIdx, Individuals.Num());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual index is not valid %d/%d, this should not happen .."),
			*FString(__FUNCTION__), __LINE__, *GetName(), ViewIdx, Individuals.Num());
	}
}

// Hide original individual, show mask clone
void ASLCVScanner::ShowMaskIndividual()
{
	if (Individuals.IsValidIndex(ViewIdx))
	{
		auto Individual = Individuals[ViewIdx];
		Individual->GetParentActor()->SetActorHiddenInGame(true);
		if (auto Clone = IndividualsMaskClones.Find(Individual))
		{
			(*Clone)->SetActorHiddenInGame(false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual clone is not found, this should not happen .."),
				*FString(__FUNCTION__), __LINE__, ViewIdx, Individuals.Num());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual index is not valid %d/%d, this should not happen .."),
			*FString(__FUNCTION__), __LINE__, ViewIdx, Individuals.Num());
	}
}

// Remove detachments and hide all actors in the world
void ASLCVScanner::SetWorldState()
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

		// Clear any attachments between actors
		ActItr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		// Hide by default
		ActItr->SetActorHiddenInGame(true);
	}

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

	for (TActorIterator<ASLIndividualManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			IndividualManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_IndividualManager");
	IndividualManager = GetWorld()->SpawnActor<ASLIndividualManager>(SpawnParams);
#if WITH_EDITOR
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
	return true;
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

// Spawn a light actor which will also be used to move the camera around
bool ASLCVScanner::SetCameraPoseAndLightActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_CameraLightAndPose");
	CameraPoseAndLightActor = GetWorld()->SpawnActor<ADirectionalLight>(SpawnParams);
#if WITH_EDITOR
	CameraPoseAndLightActor->SetActorLabel(FString(TEXT("L_CameraLightAndPose")));
#endif // WITH_EDITOR
	CameraPoseAndLightActor->SetMobility(EComponentMobility::Movable);
	CameraPoseAndLightActor->GetLightComponent()->SetIntensity(CameraLightIntensity);
	return true;
}

// Create clones of the individuals with mask material
bool ASLCVScanner::SetMaskClones()
{
	// Get the dynamic mask material
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this, DynMaskMatAssetPath);
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load default mask material.."),
			*FString(__func__), __LINE__, *GetName());
		return false;
	}
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Create the dynamic mask material and set color
	UMaterialInstanceDynamic* DynamicMaskMaterial = nullptr;
	if (!bUseIndividualMaskValue)
	{
		DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), MaskColor);
	}

	if (ScanMode == ESLCVScanMode::Individuals)
	{
		for (const auto& VI : Individuals)
		{
			if (bUseIndividualMaskValue)
			{				
				DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
				DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FColor::FromHex(VI->GetVisualMaskValue()));
			}

			if (auto AsSMA = Cast<AStaticMeshActor>(VI->GetParentActor()))
			{
				FActorSpawnParameters Parameters;
				Parameters.Template = AsSMA;
				Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				//Parameters.Instigator = SMA->GetInstigator();
				Parameters.Name = FName(*(AsSMA->GetName() + TEXT("_MaskClone")));
				AStaticMeshActor* SMAClone = GetWorld()->SpawnActor<AStaticMeshActor>(AsSMA->GetClass(), Parameters);
				if (UStaticMeshComponent* SMC = SMAClone->GetStaticMeshComponent())
				{
					for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
					{
						SMC->SetMaterial(MatIdx, DynamicMaskMaterial);
					}
				}
				SMAClone->SetActorHiddenInGame(true);
				IndividualsMaskClones.Add(VI, SMAClone);
			}
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		for (const auto& Scene : Scenes)
		{
		}
	}
	return IndividualsMaskClones.Num() > 0;
}

// Set the background static mesh actor and material
bool ASLCVScanner::SetBackgroundStaticMeshActor()
{
	// Spawn actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_BackgroundSphereMesh");
	BackgroundSMA = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
#if WITH_EDITOR
	BackgroundSMA->SetActorLabel(FString(TEXT("SM_Background")));
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
	MID->SetVectorParameterValue(FName("MaskColorParam"), BackgroundColor);
	BackgroundSM->SetMaterial(0, MID);

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

			CameraScanPoses.Emplace(Quat, Point);
		}
	}

	return CameraScanPoses.Num() > 0;
}

// Set the image name
void ASLCVScanner::SetImageName()
{
	//CurrImageName = ViewIdxString + "_" + CameraPoseIdxString + "_" + ViewModeString;
	CurrImageName = ViewModeString + "_" + ViewIdxString + "_" + CameraPoseIdxString + "_";
}

// Calculate camera pose sphere radius (proportionate to the sphere bounds of the visual mesh)
void ASLCVScanner::CalcCameraPoseSphereRadius()
{
	if (ScanMode == ESLCVScanMode::Individuals)
	{
		if (Individuals.IsValidIndex(ViewIdx))
		{
			if (auto AsSMA = Cast<AStaticMeshActor>(Individuals[ViewIdx]->GetParentActor()))
			{
				const float SphereRadius = AsSMA->GetStaticMeshComponent()->Bounds.SphereRadius;
				CurrCameraPoseSphereRadius = SphereRadius * 1.75f;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual %s is not of a supported type.."),
					*FString(__FUNCTION__), __LINE__, *GetName(), *Individuals[ViewIdx]->GetParentActor()->GetName());

				FVector BBOrigin;
				FVector BBBoxExtent;
				Individuals[ViewIdx]->GetParentActor()->GetActorBounds(false, BBOrigin, BBBoxExtent);
				CurrCameraPoseSphereRadius = BBBoxExtent.Size() * 1.75;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual index is not valid %d/%d, this should not happen .."),
				*FString(__FUNCTION__), __LINE__, *GetName(), ViewIdx, Individuals.Num());
		}
	}
	else if (ScanMode == ESLCVScanMode::Scenes)
	{
		CurrCameraPoseSphereRadius = 200.f;
	}
}

// Print progress to terminal
void ASLCVScanner::PrintProgress() const
{
	// Current scan
	int32 CurrScan = ViewIdx * CameraScanPoses.Num() * ViewModes.Num() 
		+ CameraPoseIdx * ViewModes.Num() 
		+ ViewModeIdx + 1;
	int32 NumScans = Individuals.Num() * CameraScanPoses.Num() * ViewModes.Num();	
	UE_LOG(LogTemp, Log, TEXT("%s::%d::%f Individual:\t%ld/%ld; CameraPose:\t%ld/%ld; ViewMode=\t%ld/%ld; Scan:\t%ld/%ld;"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
		ViewIdx + 1, Individuals.Num(),
		CameraPoseIdx + 1, CameraScanPoses.Num(),
		ViewModeIdx + 1, ViewModes.Num(),
		CurrScan, NumScans );
}

// Save image to file
void ASLCVScanner::SaveToFile(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap) const
{
	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, InBitmap, CompressedBitmap);
	//const FString TaskFolderPath = TaskId + "/Scans/" + ViewNameString + "/" + ViewModeString + "/";
	const FString TaskFolderPath = "/SL/" + TaskId + "/Scans/" + ViewNameString + /*"/" + ViewModeString*/ + "/";
	FString Path = FPaths::ProjectDir() + TaskFolderPath + CurrImageName + ".png";
	FPaths::RemoveDuplicateSlashes(Path);
	FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
}
