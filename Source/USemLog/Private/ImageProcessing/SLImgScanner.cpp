// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "ImageProcessing/SLImgScanner.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "FileHelper.h"
#include "Engine.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLImgScanner::ASLImgScanner()
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
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLImgScanner"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Dtor
ASLImgScanner::~ASLImgScanner()
{
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		Finish(true);
	}
}

// Called when the game starts or when spawned
void ASLImgScanner::BeginPlay()
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
void ASLImgScanner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish();
	}
}

// Set up any required references and connect to server
void ASLImgScanner::Init()
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
			VisibleIndividuals.Add(AsVI);
		}
	}
	if (VisibleIndividuals.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not find any visible individuals in the world (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *GetWorld()->GetName());
		return;
	}

	// TODO clone the visible individual parents

	/* Set the camera pose dummy actor */
	if (!SetCameraPoseAndLightActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the camera pose and light actor .."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	CameraPoseAndLightActor->SetActorTransform(FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f)));

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Start processing any incomming messages
void ASLImgScanner::Start()
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


	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Stop processing the messages, and disconnect from server
void ASLImgScanner::Finish(bool bForced)
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
void ASLImgScanner::AsyncScreenshotRequest()
{
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
		{
			GetHighResScreenshotConfig().FilenameOverride = CurrImageName;
			ViewportClient->Viewport->TakeHighResScreenShot();
		});
}

// Called when the screenshot is captured
void ASLImgScanner::ScreenshotCapturedCallback(int32 SizeX, int32 SizeY, const TArray<FColor>& InBitmap)
{
	//// Check if the image should be stored locally
	//if (bSaveToFile)
	//{
	//	// Compress image
	//	TArray<uint8> CompressedBitmap;
	//	FImageUtils::CompressImageArray(SizeX, SizeY, InBitmap, CompressedBitmap);
	//	FString Path = FPaths::ProjectDir() + "/SL/" + TaskId + "/CalibratedMasks/" + CurrImageName + ".png";
	//	FPaths::RemoveDuplicateSlashes(Path);
	//	FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	//}

	//// Set the individuals calibrated/rendered color
	//if (VisibleIndividuals.IsValidIndex(IndividualIdx))
	//{
	//	USLVisibleIndividual* VI = VisibleIndividuals[IndividualIdx];
	//	VI->SetCalibratedVisualMaskValue(GetCalibratedColorMask(InBitmap));
	//	if (ApplyChangesToEditorIndividual(VI))
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("%s::%d::%.4f\t[%d/%d]\t%s\t calibrated:\t%s->%s;"),
	//			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), IndividualIdx, VisibleIndividuals.Num(),
	//			*VI->GetParentActor()->GetName(), *VI->GetVisualMaskValue(), *VI->GetCalibratedVisualMaskValue());
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Index %d/%d is not valid, this should not happen.."),
	//		*FString(__FUNCTION__), __LINE__, IndividualIdx, VisibleIndividuals.Num());
	//	return;
	//}

	//// Goto next individual
	//if (SetNextIndividual())
	//{
	//	AsyncScreenshotRequest();
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s finished, quitting editor.."),
	//		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName());
	//	QuitEditor();
	//}
}

// Hide all actors in the world
void ASLImgScanner::HideAllActors()
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		ActItr->SetActorHiddenInGame(true);
	}
}

// Get the individual manager from the world (or spawn a new one)
bool ASLImgScanner::SetIndividualManager()
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

// Spawn a light actor which will also be used to move the camera around
bool ASLImgScanner::SetCameraPoseAndLightActor()
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
