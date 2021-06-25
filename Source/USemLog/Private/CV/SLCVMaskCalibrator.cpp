// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVMaskCalibrator.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameViewportClient.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "FileHelper.h"
#include "Engine.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLCVMaskCalibrator::ASLCVMaskCalibrator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIgnore = true;
	bSaveToFile = false;
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLCVMaskCalibrator"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Dtor
ASLCVMaskCalibrator::~ASLCVMaskCalibrator()
{
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		Finish(true);
	}
}

// Called when the game starts or when spawned
void ASLCVMaskCalibrator::BeginPlay()
{
	Super::BeginPlay();

	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's ignore flag is true, skipping.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	Init();
	Start();
}

// Called when actor removed from game or game ended
void ASLCVMaskCalibrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		Finish();
	}
}

// Set up any required references and connect to server
void ASLCVMaskCalibrator::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already initialized.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	
	// Hide everything in the world
	HideAllActors();

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

	// Get the visible individuals
	for (const auto& BI : IndividualManager->GetIndividuals())
	{
		if (auto AsVI = Cast<USLVisibleIndividual>(BI))
		{
			Individuals.Add(AsVI);
		}
	}
	if (Individuals.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not find any visible individuals in the world (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *GetWorld()->GetName());
		return;
	}

	/* Set the canvas actor */
	if (!SetCanvasMeshActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the canvas mesh actor .."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	CanvasSMA->SetActorTransform(FTransform(FRotator(0.f, 0.f, 0.f), FVector(150.f, 0.f, 0.f)));

	/* Set the camera pose dummy actor */
	if (!SetCameraPoseActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the camera pose actor .."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	CameraPoseActor->SetActorTransform(FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f)));

	/* Set render and screenshot params */
	SetScreenshotResolution(FIntPoint(40, 30));
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
	ViewportClient->OnScreenshotCaptured().AddUObject(this, &ASLCVMaskCalibrator::ScreenshotCapturedCallback);

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Start processing any incomming messages
void ASLCVMaskCalibrator::Start()
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

	// Set view pose	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		// Hide default pawn
		PC->GetPawnOrSpectator()->SetActorHiddenInGame(true);

		// Set view mode to unlit
		PC->ConsoleCommand("viewmode unlit");

		// Set view target to dummy camera
		PC->SetViewTarget(CameraPoseActor);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot access the first player controller.."), *FString(__func__), __LINE__);
		return;
	}

	// Set the first individual
	if (!SetNextView())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set the first individual.."), *FString(__func__), __LINE__);
		return;
	}

	// Start the dominoes
	RequestScreenshotAsync();

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Stop processing the messages, and disconnect from server
void ASLCVMaskCalibrator::Finish(bool bForced)
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


	// Start calibrating the individuals
	ViewIdx = 0;
	SetActorTickEnabled(false);

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully finished.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Request a high res screenshot
void ASLCVMaskCalibrator::RequestScreenshotAsync()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
		{
			GetHighResScreenshotConfig().FilenameOverride = CurrImageName;
			ViewportClient->Viewport->TakeHighResScreenShot();
		});
}

// Called when the screenshot is captured
void ASLCVMaskCalibrator::ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap)
{
	// Check if the image should be stored locally
	if (bSaveToFile)
	{
		// Compress image
		TArray<uint8> CompressedBitmap;
		FImageUtils::CompressImageArray(SizeX, SizeY, InBitmap, CompressedBitmap);
		FString Path = FPaths::ProjectDir() + "/SL/" + TaskId + "/CalibratedMasks/" + CurrImageName + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	// Set the individuals calibrated/rendered color
	if (Individuals.IsValidIndex(ViewIdx))
	{
		USLVisibleIndividual* VI = Individuals[ViewIdx];
		VI->SetCalibratedVisualMaskValue(GetCalibratedMask(InBitmap));
		if (ApplyChangesToEditorIndividual(VI))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d::%.4f\t[%d/%d]\t%s\t calibrated:\t%s->%s;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), ViewIdx, Individuals.Num(),
				*VI->GetParentActor()->GetName(), *VI->GetVisualMaskValue(), *VI->GetCalibratedVisualMaskValue());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Index %d/%d is not valid, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, ViewIdx, Individuals.Num());
		return;
	}

	// Goto next individual
	if (SetNextView())
	{
		RequestScreenshotAsync();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s finished, quitting editor.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName());
		QuitEditor();
	}
}

// Set the next individual to calibrate
bool ASLCVMaskCalibrator::SetNextView()
{
	ViewIdx++;
	if (Individuals.IsValidIndex(ViewIdx))
	{
		auto VI = Individuals[ViewIdx];

		// Set image name
		CurrImageName = FString::FromInt(ViewIdx)
			+ "_" + FString::FromInt(Individuals.Num())
			+ VI->GetIdValue();

		// Get the color from the individual
		FColor MaskColor = FColor::FromHex(VI->GetVisualMaskValue());
		if (MaskColor == FColor::Black)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %'s mask color (%s) is black.."),
				*FString(__FUNCTION__), __LINE__, *VI->GetParentActor()->GetName(), *VI->GetVisualMaskValue());
		}

		// Apply color to canvas
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
#if WITH_EDITOR	
		CanvasSM->SetMaterial(0, DynamicMaskMaterial);
#endif
		return true;
	}
	else
	{
		ViewIdx = INDEX_NONE;
		return false;
	}
}

// Get the calibrated color from the rendered screenshot image
FString ASLCVMaskCalibrator::GetCalibratedMask(const TArray<FColor>& Bitmap)
{
	bool bRenderedColorIsSet = false;
	FColor RenderedColor;

	for (const auto& C : Bitmap)
	{
		if (C != FColor::Black)
		{
			if (bRenderedColorIsSet)
			{
				if (C != RenderedColor)
				{
					// Make sure no other nuances appear
					UE_LOG(LogTemp, Error, TEXT("%s::%d Different color nuance found %s/%s;"),
						*FString(__func__), __LINE__, *C.ToString(), *RenderedColor.ToString());
				}
			}
			else
			{
				RenderedColor = C;
				bRenderedColorIsSet = true;
			}
		}
	}
	return RenderedColor.ToHex();
}

// Apply changes to the editor individual
bool ASLCVMaskCalibrator::ApplyChangesToEditorIndividual(USLVisibleIndividual* VisibleIndividual)
{
	// Apply the changes in the editor world
#if WITH_EDITOR
	if (AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(VisibleIndividual->GetParentActor()))
	{
		if (auto BI = FSLIndividualUtils::GetIndividualObject(EdAct))
		{
			if (auto VI = Cast<USLVisibleIndividual>(BI))
			{
				VI->SetCalibratedVisualMaskValue(VisibleIndividual->GetCalibratedVisualMaskValue());
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual is not of type visible.."),
					*FString(__func__), __LINE__, *VisibleIndividual->GetParentActor()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s's individual in the editor world.."),
				*FString(__func__), __LINE__, *VisibleIndividual->GetParentActor()->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in the editor world.."),
			*FString(__func__), __LINE__, *VisibleIndividual->GetParentActor()->GetName());
	}
#endif // WITH_EDITOR
	return false;
}

//  Quit the editor when finished
void ASLCVMaskCalibrator::QuitEditor()
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

// Hide all actors in the world
void ASLCVMaskCalibrator::HideAllActors()
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		ActItr->SetActorHiddenInGame(true);
	}
}

// Set screenshot image resolution
void ASLCVMaskCalibrator::SetScreenshotResolution(FIntPoint Resolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	// !! Workaround !! Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;
}

// Set the rendering parameters
void ASLCVMaskCalibrator::SetRenderParams()
{
	// Defines the memory layout used for the GBuffer,
	// 0: lower precision (8bit per component, for profiling), 1: low precision (default)
	// 3: high precision normals encoding, 5: high precision
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.GBufferFormat"))->Set(5);


	// Set the near clipping plane (in cm)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetNearClipPlane"))->Set(0); // Not a console variable, but a command
	//GNearClippingPlane = 0; // View is distorted after finishing the scanning
	//#if WITH_EDITOR
	//	if (GEngine)
	//	{
	//		GEngine->DeferredCommands.Add(TEXT("r.SetNearClipPlane 0"));
	//	}
	//#endif // WITH_EDITOR

	//// AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_None);

	//// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// LOD level to force, -1 is off. (0 - Best)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Get the individual manager from the world (or spawn a new one)
bool ASLCVMaskCalibrator::SetIndividualManager()
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

// Spawn the canvas static mesh actor (mesh on which the mask colors will be applied)
bool ASLCVMaskCalibrator::SetCanvasMeshActor()
{
	// Spawn actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_CalibrateCanvasMesh");
	CanvasSMA = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
#if WITH_EDITOR
	CanvasSMA->SetActorLabel(FString(TEXT("SM_CalibrateCanvasMesh")));
#endif // WITH_EDITOR

	// Set the mesh component
	CanvasSM = LoadObject<UStaticMesh>(nullptr, CanvasSMAssetPath);
	if (!CanvasSM)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load canvas mesh.."), *FString(__func__), __LINE__);
		CanvasSMA->Destroy();
		return false;
	}
	CanvasSMA->GetStaticMeshComponent()->SetStaticMesh(CanvasSM);
	CanvasSMA->SetMobility(EComponentMobility::Movable);


	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this, DynMaskMatAssetPath);
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."), *FString(__func__), __LINE__);
		CanvasSMA->Destroy();
		return false;
	}
	//DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
	DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::White);
#if WITH_EDITOR	
	CanvasSM->SetMaterial(0, DynamicMaskMaterial);
#endif
	return true;
}

// Spawn a dummy actor to move the camera to
bool ASLCVMaskCalibrator::SetCameraPoseActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_CalibCameraPoseDummy");
	CameraPoseActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
#if WITH_EDITOR
	CameraPoseActor->SetActorLabel(FString(TEXT("SM_CalibrateCameraPoseDummy")));
#endif // WITH_EDITOR

	// Set the mesh component
	UStaticMesh* CameraPoseDummyMesh = LoadObject<UStaticMesh>(nullptr, CameraDummySMAssetPath);
	if (!CameraPoseDummyMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find camera pose dummy mesh.."), *FString(__func__), __LINE__);
		CameraPoseActor->Destroy();
		return false;
	}
	CameraPoseActor->GetStaticMeshComponent()->SetStaticMesh(CameraPoseDummyMesh);
	CameraPoseActor->SetMobility(EComponentMobility::Movable);

	return true;
}

