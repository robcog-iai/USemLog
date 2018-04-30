// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRuntimeManager.h"
#include "SLRawDataLogger.h"
#include "Ids.h"

// Sets default values
ASLRuntimeManager::ASLRuntimeManager()
{
	// Disable tick on actor
	PrimaryActorTick.bCanEverTick = false;

	// Flags
	bIsInit = false;
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

// Sets default values
ASLRuntimeManager::~ASLRuntimeManager()
{
	ASLRuntimeManager::Stop();
}

// Called when the game starts or when spawned
void ASLRuntimeManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		ASLRuntimeManager::Init();
		ASLRuntimeManager::Start();
	}
}

// Called when actor removed from game or game ended
void ASLRuntimeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ASLRuntimeManager::Stop();
}

// Init manager
void ASLRuntimeManager::Init()
{
	if (!bIsInit)
	{
		if (EpisodeId.Equals(TEXT("autogen")))
		{
			// Generate unique id for the episode
			EpisodeId = FIds::NewGuidInBase64Url();
		}
				
		if (bLogRawData)
		{
			// Create and init raw data logger
			RawDataLogger = NewObject<USLRawDataLogger>(this);
			RawDataLogger->Init(DistanceThreshold);

			// Set log type
			if (bLogToJson)
			{
				RawDataLogger->SetLogToJson(LogDirectory, EpisodeId);
			}
			if (bLogToBson)
			{
				RawDataLogger->SetLogToBson(LogDirectory, EpisodeId);
			}
			if (bLogToMongo)
			{
				RawDataLogger->SetLogToMongo(LogDirectory, EpisodeId, MongoIP, MongoPort);
			}
		}

		// Mark manager as initialized
		bIsInit = true;
	}
}

// Start manager
void ASLRuntimeManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Start raw data logger
		if (bLogRawData)
		{
			RawDataLogger->Start(UpdateRate);
		}

		// Mark manager as started
		bIsStarted = true;
	}
}

// Stop manager
void ASLRuntimeManager::Stop()
{
	if (bIsStarted)
	{
		if (RawDataLogger)
		{
			RawDataLogger->Stop();
		}

		// Set manager as stopped
		bIsStarted = false;
		bIsInit = false;
	}
}
