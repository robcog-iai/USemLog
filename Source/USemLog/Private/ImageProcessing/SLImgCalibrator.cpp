// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "ImageProcessing/SLImgCalibrator.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLImgCalibrator::ASLImgCalibrator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bIgnore = true;
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLImgCalibrator"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Dtor
ASLImgCalibrator::~ASLImgCalibrator()
{
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		Finish(true);
	}
}

// Called when the game starts or when spawned
void ASLImgCalibrator::BeginPlay()
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

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLImgCalibrator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	//if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLImgCalibrator, bTriggerButtonHack))
	//{
	//	bTriggerButtonHack = false;
	//}
}
#endif // WITH_EDITOR

// Called every frame
void ASLImgCalibrator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CalibrateIndividual())
	{
		IndividualIdx++;
	}
	else
	{
		IndividualIdx = INDEX_NONE;
		SetActorTickEnabled(false);
	}
}

// Called when actor removed from game or game ended
void ASLImgCalibrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish();
	}
}

// Set up any required references and connect to server
void ASLImgCalibrator::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already initialized.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

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

	// Cache the visual individuals	
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

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Start processing any incomming messages
void ASLImgCalibrator::Start()
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

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		// Hide default pawn
		PC->GetPawnOrSpectator()->SetActorHiddenInGame(true);

		// Set view mode to unlit
		PC->ConsoleCommand("viewmode unlit");

		// Set view target to dummy camera
		//PC->SetViewTarget(CameraPoseActor);
	}

	// Start calibrating the individuals
	IndividualIdx = 0;
	SetActorTickEnabled(true);

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Stop processing the messages, and disconnect from server
void ASLImgCalibrator::Finish(bool bForced)
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
	IndividualIdx = 0;
	SetActorTickEnabled(false);

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully finished.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Set and calibrate the next individual (return false if there are no more indvidiuals)
bool ASLImgCalibrator::CalibrateIndividual()
{
	if (VisibleIndividuals.IsValidIndex(IndividualIdx))
	{
		USLVisibleIndividual* VI = VisibleIndividuals[IndividualIdx];
		UE_LOG(LogTemp, Log, TEXT("%s::%d::%.4f Calibrating %s, %s->%s"),
			*FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *VI->GetParentActor()->GetName(),
			*VI->GetVisualMaskValue(), *VI->GetCalibratedVisualMaskValue());
		return true;
	}
	else
	{
		return false;
	}
}

// Get the individual manager from the world (or spawn a new one)
bool ASLImgCalibrator::SetIndividualManager()
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
