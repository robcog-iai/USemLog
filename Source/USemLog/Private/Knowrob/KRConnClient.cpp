// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "KRConnClient.h"
#include "Individuals/SLIndividualManager.h"
#include "EngineUtils.h"

// Sets default values
AKRConnClient::AKRConnClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
	bIndividualManagerSet = false;
	IndividualManager = nullptr;
}

// Called when the game starts or when spawned
void AKRConnClient::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKRConnClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AKRConnClient::Init()
{
	if (bIsInit)
	{
		return true;
	}

	if (SetIndividualManager())
	{
		bIsInit = true;
		return true;
	}
	
	return false;
}

// Reset references
void AKRConnClient::Reset()
{
	bIsInit = false;
	bIndividualManagerSet = false;
	IndividualManager = nullptr;
}

// Set the individual manager
bool AKRConnClient::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager is already set.."), *FString(__FUNCTION__), __LINE__);
		IndividualManager->Init();
		bIndividualManagerSet = true;
		return true;
	}
	for (TActorIterator<ASLIndividualManager> IMIter(GetWorld()); IMIter; ++IMIter)
	{
		IndividualManager = *IMIter;
		IndividualManager->Init();
		bIndividualManagerSet = true;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the individual manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

