// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SemanticLogger.h"
#include "Ids.h"

// Sets default values
ASemanticLogger::ASemanticLogger()
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
	EventsTemplateType = EEventsTemplate::Default;
}

// Sets default values
ASemanticLogger::~ASemanticLogger()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	ASemanticLogger::FinishLogging();
}

// Called when the game starts or when spawned
void ASemanticLogger::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		ASemanticLogger::InitLogging();
		ASemanticLogger::StartLogging();
	}
}

// Called when actor removed from game or game ended
void ASemanticLogger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ASemanticLogger::FinishLogging();
}

// 
void ASemanticLogger::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

//
void ASemanticLogger::FinishDestroy()
{
	Super::FinishDestroy();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Init loggers
void ASemanticLogger::InitLogging()
{
	if (!bIsInit)
	{
		// Init the semantic items content singleton
		FSLContentSingleton::GetInstance()->Init();

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
void ASemanticLogger::StartLogging()
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
void ASemanticLogger::FinishLogging()
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
void ASemanticLogger::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Radio button style between bLogToJson, bLogToBson, bLogToMongo
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASemanticLogger, bLogToJson)) 
	{
		if (bLogToJson)
		{
			// Set bLogToBson and bLogToMongo to false (radio button style)
			bLogToBson = false;
			bLogToMongo = false;
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASemanticLogger, bLogToBson))
	{
		if (bLogToBson)
		{
			bLogToJson = false;
			bLogToMongo = false;
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASemanticLogger, bLogToMongo))
	{
		if (bLogToMongo)
		{
			bLogToJson = false;
			bLogToBson = false;
		}
	}
}
#endif // WITH_EDITOR