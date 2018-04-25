// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRuntimeManager.h"
#include "SLRawDataLogger.h"
#include "Ids.h"

// Sets default values
ASLRuntimeManager::ASLRuntimeManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// Flags
	bIsStarted = false;

	// Semantic logger default values
	EpisodeId = TEXT("autogen");
	LogDirectory = TEXT("SemLog");
	bStartAtBeginPlay = true;

	// Raw data logger default values
	bLogRawData = true;
	UpdateRate = 0.0f;
	DistanceThreshold = 0.5f;
	bLogToJson = true;
	bLogToBson = false;
	bLogToMongo = false;
	MongoIP = TEXT("127.0.0.1.");
	MongoPort = 27017;	
}

// Called when the game starts or when spawned
void ASLRuntimeManager::BeginPlay()
{
	Super::BeginPlay();

	if (EpisodeId.Equals(TEXT("autogen")))
	{
		EpisodeId = FIds::NewGuidInBase64Url();
	}

	if (bStartAtBeginPlay)
	{
		Start();
	}
}

// Called when actor removed from game or game ended
void ASLRuntimeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Start manager
void ASLRuntimeManager::Start()
{
	if (!bIsStarted)
	{
		if (bLogRawData)
		{
			RawDataLogger = NewObject<USLRawDataLogger>(this, TEXT("RawDataLogger"));
			if (bLogToJson)
			{
				RawDataLogger->LogToJson(true);
			}
			RawDataLogger->Start(GetWorld(), LogDirectory, EpisodeId, UpdateRate, DistanceThreshold);
		}
		bIsStarted = true;
	}
}
