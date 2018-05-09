// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRuntimeManager.h"
#include "RawData/SLRawDataLogger.h"
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

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLRuntimeManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Radio button style between bLogToJson, bLogToBson, bLogToMongo
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLRuntimeManager, bLogToJson)) 
	{
		if (bLogToJson)
		{
			// Set bLogToBson and bLogToMongo to false (radio button style)
			bLogToBson = false;
			bLogToMongo = false;
		}
		UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLRuntimeManager, bLogToBson))
	{
		if (bLogToBson)
		{
			bLogToJson = false;
			bLogToMongo = false;
		}
		UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLRuntimeManager, bLogToMongo))
	{
		if (bLogToMongo)
		{
			bLogToJson = false;
			bLogToBson = false;
		}
		UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	}
}
#endif // WITH_EDITOR