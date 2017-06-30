// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRuntimeManager.h"


// Sets default values
ASLRuntimeManager::ASLRuntimeManager()
{
 	// No tick by default
	PrimaryActorTick.bCanEverTick = false;
	
	// Flag defaults
	bLogRawData = true;
	bLogEventData = true;

	// Create UObjects
	EventLogger = CreateDefaultSubobject<USLEventData>(TEXT("EventDataLogger"));
	RawDataLogger = CreateDefaultSubobject<USLRawData>(TEXT("RawDataLogger"));
}

// Called when the game starts or when spawned
void ASLRuntimeManager::BeginPlay()
{
	Super::BeginPlay();

	if (bLogRawData && RawDataLogger)
	{
		// Enable tick for raw data logging
		SetActorTickEnabled(true);

		// Initalize the raw data logger and log the static and dynamic entities
		RawDataLogger->InitAndFirstLog(GetWorld());
	}	
}

// Called every frame
void ASLRuntimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Log the raw data of the dynamic entities
	RawDataLogger->LogDynamicEntities();
}

