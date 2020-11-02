// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizSemMapManager.h"
#include "Individuals/SLIndividualManager.h"
#include "Viz/SLVizManager.h"
#include "TimerManager.h"
#include "EngineUtils.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLVizSemMapManager::ASLVizSemMapManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
	bExecutingTask = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLVizSemMap"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLVizSemMapManager::BeginPlay()
{
	Super::BeginPlay();

	//Init();
	//Start();
}

// Called every frame
void ASLVizSemMapManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when actor removed from game or game ended
void ASLVizSemMapManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Set up references and the world
void ASLVizSemMapManager::Init()
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
	if (!IndividualManager->Load(false))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		return;
	}

	if (!SetVizManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not get access to the viz manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	VizManager->Init();
	if (!VizManager->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not init the viz manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	VizManager->ConvertWorldToVisualizationMode();

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// 
void ASLVizSemMapManager::Reset()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, nothing to reset.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}


	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Get the individual manager from the world (or spawn a new one)
bool ASLVizSemMapManager::SetIndividualManager()
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

// Get the viz manager from the world (or spawn a new one)
bool ASLVizSemMapManager::SetVizManager()
{
	if (VizManager && VizManager->IsValidLowLevel() && !VizManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			VizManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizManager");
	VizManager = GetWorld()->SpawnActor<ASLVizManager>(SpawnParams);
#if WITH_EDITOR
	VizManager->SetActorLabel(TEXT("SL_VizManager"));
#endif // WITH_EDITOR
	return true;
}

