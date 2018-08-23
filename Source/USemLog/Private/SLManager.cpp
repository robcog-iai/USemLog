// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManager.h"
#include "SLMappings.h"
#include "SLContentSingleton.h"
#include "Ids.h"

// Sets default values
ASLManager::ASLManager()
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

	// Events logger default values
	bLogEventData = true;
	EventsTemplateType = ESLEventsTemplate::Default;
}

// Sets default values
ASLManager::~ASLManager()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	ASLManager::Finish();
}

// Called when the game starts or when spawned
void ASLManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		ASLManager::Init();
		ASLManager::Start();
	}
}

// Called when actor removed from game or game ended
void ASLManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ASLManager::Finish();
}

// 
void ASLManager::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

//
void ASLManager::FinishDestroy()
{
	Super::FinishDestroy();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Init loggers
void ASLManager::Init()
{
	if (!bIsInit)
	{
		// Init the semantic items content singleton
		//FSLMappings::GetInstance()->LoadData(GetWorld());
		//FSLMappings::GetInstance();
		//FSLContentSingleton::GetInstance();

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

		if (bLogEventData)
		{
			// Create and init event data logger
			EventDataLogger = NewObject<USLEventDataLogger>(this);
			EventDataLogger->Init(LogDirectory, EpisodeId, EventsTemplateType);
		}

		// Mark manager as initialized
		bIsInit = true;
	}
}

// Start loggers
void ASLManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Start raw data logger
		if (bLogRawData && RawDataLogger)
		{
			RawDataLogger->Start(UpdateRate);
		}

		// Start event data logger
		if (bLogEventData && EventDataLogger)
		{
			EventDataLogger->Start();
		}

		// Mark manager as started
		bIsStarted = true;
	}
}

// Finish loggers
void ASLManager::Finish()
{
	if (bIsStarted || bIsInit)
	{
		if (RawDataLogger)
		{
			RawDataLogger->Finish();
		}

		if (EventDataLogger)
		{
			EventDataLogger->Finish();
		}

		// Delete the semantic items content instance
		FSLContentSingleton::DeleteInstance();

		// Mark manager as finished
		bIsStarted = false;
		bIsInit = false;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Radio button style between bLogToJson, bLogToBson, bLogToMongo
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogToJson)) 
	{
		if (bLogToJson)
		{
			// Set bLogToBson and bLogToMongo to false (radio button style)
			bLogToBson = false;
			bLogToMongo = false;
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogToBson))
	{
		if (bLogToBson)
		{
			bLogToJson = false;
			bLogToMongo = false;
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogToMongo))
	{
		if (bLogToMongo)
		{
			bLogToJson = false;
			bLogToBson = false;
		}
	}
}
#endif // WITH_EDITOR