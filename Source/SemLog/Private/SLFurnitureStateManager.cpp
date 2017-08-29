// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLFurnitureStateManager.h"

// Sets default values
ASLFurnitureStateManager::ASLFurnitureStateManager()
{
	PrimaryActorTick.bCanEverTick = false;

	UpdateRate = 0.25f;
}

// Called when the game starts or when spawned
void ASLFurnitureStateManager::BeginPlay()
{
	Super::BeginPlay();

	// Get the semantic log runtime manager from the world
	for (TActorIterator<ASLRuntimeManager>RMItr(GetWorld()); RMItr; ++RMItr)
	{
		SemLogRuntimeManager = *RMItr;
		break;
	}

	if (SemLogRuntimeManager)
	{
		// Check drawer states with the given update rate (add delay until the drawers are closed)
		GetWorldTimerManager().SetTimer(FurnitureStateTimerHandle, this, &ASLFurnitureStateManager::CheckFurnitureState, UpdateRate, true);
	}
}

// Check drawer states
void ASLFurnitureStateManager::CheckFurnitureState()
{
	UE_LOG(LogTemp, Warning, TEXT("Checking furniture state"));
}

