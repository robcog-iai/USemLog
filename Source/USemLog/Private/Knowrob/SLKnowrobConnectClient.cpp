// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKnowrobConnectClient.h"
#include "Individuals/SLIndividualManager.h"
#include "Mongo/SLMongoManager.h"
#include "Viz/SLVizManager.h"
#include "EngineUtils.h"

// Sets default values
ASLKnowrobConnectClient::ASLKnowrobConnectClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;

	bIndividualManagerSet = false;
	bMongoManagerSet = false;
	bVizManagerSet = false;

	IndividualManager = nullptr;
	MongoManager = nullptr;
	VizManager = nullptr;
}

// Called when the game starts or when spawned
void ASLKnowrobConnectClient::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ASLKnowrobConnectClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ASLKnowrobConnectClient::Init()
{
	if (bIsInit)
	{
		return true;
	}

	if (SetIndividualManager() && SetMongoManager() && SetVizManager())
	{
		bIsInit = true;
		return true;
	}
	
	return false;
}

// Reset references
void ASLKnowrobConnectClient::Reset()
{
	bIsInit = false;
	
	bIndividualManagerSet = false;
	bMongoManagerSet = false;
	bVizManagerSet = false;

	IndividualManager = nullptr;
	MongoManager = nullptr;
	VizManager = nullptr;
}

// Set the individual manager
bool ASLKnowrobConnectClient::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager is already set.."), *FString(__FUNCTION__), __LINE__);
		IndividualManager->Init();
		bIndividualManagerSet = true;
		return true;
	}
	for (TActorIterator<ASLIndividualManager> Iter(GetWorld()); Iter; ++Iter)
	{
		IndividualManager = *Iter;
		IndividualManager->Init();
		bIndividualManagerSet = true;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the individual manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

// Set the mongo db manager
bool ASLKnowrobConnectClient::SetMongoManager()
{
	if (MongoManager && MongoManager->IsValidLowLevel() && !MongoManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Mongo manager is already set.."), *FString(__FUNCTION__), __LINE__);
		//MongoManager->Init();
		bMongoManagerSet = true;
		return true;
	}
	for (TActorIterator<ASLMongoManager> Iter(GetWorld()); Iter; ++Iter)
	{
		MongoManager = *Iter;
		//MongoManager->Init();
		bMongoManagerSet = true;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the mongo manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

// Set the visualization manager
bool ASLKnowrobConnectClient::SetVizManager()
{
	if (VizManager && VizManager->IsValidLowLevel() && !VizManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager is already set.."), *FString(__FUNCTION__), __LINE__);
		//VizManager->Init();
		bVizManagerSet = true;
		return true;
	}
	for (TActorIterator<ASLVizManager> Iter(GetWorld()); Iter; ++Iter)
	{
		VizManager = *Iter;
		//VizManager->Init();
		bVizManagerSet = true;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the viz manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

