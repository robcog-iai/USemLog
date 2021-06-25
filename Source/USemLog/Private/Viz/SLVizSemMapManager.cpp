// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizSemMapManager.h"
#include "Individuals/SLIndividualManager.h"
#include "Viz/SLVizManager.h"
#include "Individuals/Type/SLBaseIndividual.h"
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
	if (!IndividualManager->IsLoaded() && !IndividualManager->Load(true))
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
	//VizManager->ConvertWorldToVisualizationMode();

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

// Hide/show all individuals in the world
void ASLVizSemMapManager::SetAllIndividualsHidden(bool bNewHidden)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not initialized, init first.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	for (const auto Individual : IndividualManager->GetIndividuals())
	{
		if (AActor* ParentActor = Individual->GetParentActor())
		{
			ParentActor->SetActorHiddenInGame(bNewHidden);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Individual %s has no valid parent actor.."), *FString(__FUNCTION__), __LINE__,
				*Individual->GetFullName());
		}
	}
}


// Hide/show selected individuals
void ASLVizSemMapManager::SetIndividualsHidden(const TArray<FString>& Ids, bool bNewHidden,
	bool bIterate, float IterateInterval)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not initialized, init first.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	
	// Check if there are any running iterations
	if (GetWorld()->GetTimerManager().IsTimerActive(IterateTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f %s's timer is still active, terminating timer and finishing up task.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName());

		// Deactivate timer, and finish the task
		GetWorld()->GetTimerManager().ClearTimer(IterateTimerHandle);
		
		// Finish up previous work
		for (; IterateIdx < IterateIds.Num(); ++IterateIdx)
		{
			IndividualManager->GetIndividualActor(IterateIds[IterateIdx])->SetActorHiddenInGame(bIterateHiddenValue);
		}
		IterateIdx = INDEX_NONE;
		IterateIds.Empty();
	}

	// Execute as iteration with given interval or all at once
	if (bIterate && IterateInterval > 0.f)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d::%.4f %s's iter timer started.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName());
		IterateIdx = 0;
		IterateIds = Ids;	
		bIterateHiddenValue = bNewHidden;
		GetWorld()->GetTimerManager().SetTimer(IterateTimerHandle,
			this, &ASLVizSemMapManager::SetIndividualsHiddenIterateCallback, IterateInterval, true);
	}
	else
	{
		for (const auto& Id : Ids)
		{
			IndividualManager->GetIndividualActor(Id)->SetActorHiddenInGame(bNewHidden);
		}
	}
}

// Iterate callback
void ASLVizSemMapManager::SetIndividualsHiddenIterateCallback()
{
	if (IterateIds.IsValidIndex(IterateIdx))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d::%.4f %s's iter timer trigger %d/%d.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName(),
			IterateIdx, IterateIds.Num());
		IndividualManager->GetIndividualActor(IterateIds[IterateIdx])->SetActorHiddenInGame(bIterateHiddenValue);
		IterateIdx++;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d::%.4f %s's iter timer reached end %d/%d.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetName(),
			IterateIdx, IterateIds.Num());
		GetWorld()->GetTimerManager().ClearTimer(IterateTimerHandle);
		IterateIdx = INDEX_NONE;
		IterateIds.Empty();
	}
}

// Spawn or get manager from the world
ASLVizSemMapManager* ASLVizSemMapManager::GetExistingOrSpawnNew(UWorld* World)
{
	// Check in world
	for (TActorIterator<ASLVizSemMapManager>Iter(World); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			return *Iter;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizSemMapManager");
	auto Manager = World->SpawnActor<ASLVizSemMapManager>(SpawnParams);
#if WITH_EDITOR
	Manager->SetActorLabel(TEXT("SL_VizSemMapManager"));
#endif // WITH_EDITOR
	return Manager;
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

