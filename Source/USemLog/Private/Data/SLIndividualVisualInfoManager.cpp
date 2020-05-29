// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfoManager.h"
#include "Data/SLIndividualVisualInfo.h"

// Sets default values
ASLIndividualVisualInfoManager::ASLIndividualVisualInfoManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;

}

// Called when the game starts or when spawned
void ASLIndividualVisualInfoManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLIndividualVisualInfoManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Load components from world
bool ASLIndividualVisualInfoManager::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
		VisualComponents.Empty();
	}

	if (!bIsInit)
	{
		bIsInit = true;
		return true;
	}

	return false;
}

// Remove destroyed individuals from array
void ASLIndividualVisualInfoManager::OnIndividualDestroyed(USLIndividualVisualInfo* Component)
{
	int32 Index = INDEX_NONE;
	if (VisualComponents.Find(Component, Index))
	{
		VisualComponents.RemoveAt(Index);
	}
}
