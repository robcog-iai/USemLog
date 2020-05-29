// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualManager.h"
#include "Data/SLIndividualComponent.h"

// Sets default values
ASLIndividualManager::ASLIndividualManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
}

// Called when the game starts or when spawned
void ASLIndividualManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLIndividualManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Load components from world
bool ASLIndividualManager::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
		IndividualComponents.Empty();
	}

	if (!bIsInit)
	{
		bIsInit = true;
		return true;
	}

	return false;
}

// Remove destroyed individuals from array
void ASLIndividualManager::OnIndividualDestroyed(USLIndividualComponent* Component)
{
	int32 Index = INDEX_NONE;
	if (IndividualComponents.Find(Component, Index))
	{
		IndividualComponents.RemoveAt(Index);
	}
}

