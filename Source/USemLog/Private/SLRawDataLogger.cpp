// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataLogger.h"
#include "Tags.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

// Constructor
USLRawDataLogger::USLRawDataLogger()
{
}

// Destructor
USLRawDataLogger::~USLRawDataLogger()
{
	// Delete async worker
	if (RawDataLogWorker)
	{
		RawDataLogWorker->EnsureCompletion();
		delete RawDataLogWorker;
	}
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Log data to json file
void USLRawDataLogger::LogToJson(bool bInLogToJson)
{
	bLogToJson = bInLogToJson;
}

// Log data to bson file
void USLRawDataLogger::LogToBson(bool bInLogToBson)
{
	bLogToBson = bInLogToBson;
}

// Log data to mongodb
void USLRawDataLogger::LogToMongo(bool bInLogToMongo, const FString& InMongoIP, uint16 MongoPort)
{
	bLogToMongo = bInLogToMongo;
}

// Start logger
void USLRawDataLogger::Start(UWorld* InWorld, const FString& InLogDirectory, const FString& InEpisodeId, const float UpdateRate, const float InDistanceThreshold)
{
	World = InWorld;
	DistanceThresholdSquared = InDistanceThreshold * InDistanceThreshold;
	
	// Set logger to update on tick or on custom update rate using a timer
	SetLoggerUpdateRate(UpdateRate);
	
	// TODO move to LogToJson/Bson/Mongo
	// Set file handle for writing data to file
	if (bLogToJson || bLogToBson)
	{
		SetLoggerFileHandle(InLogDirectory, InEpisodeId);
	}
	// Set connection to mongodb
	if (bLogToMongo)
	{
		// SetLoggerMongoConnection(
	}

	LogInitialState();
}

// Set update rate by binding to tick or a custom update rate using a timer callback
void USLRawDataLogger::SetLoggerUpdateRate(const float UpdateRate)
{
	// Set logger to update on tick or on custom update rate using a timer
	if (UpdateRate > 0.0f)
	{
		if (World)
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, this, &USLRawDataLogger::TimerCallback, UpdateRate, true);
		}
	}
	else
	{
		bIsTickable = true;
	}
}

// Set the file handle for the logger
void USLRawDataLogger::SetLoggerFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = TEXT("RawData_") + InEpisodeId + TEXT(".json");
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);
}

// Timer callback (timer tick)
void USLRawDataLogger::TimerCallback()
{
	//UE_LOG(LogTemp, Error, TEXT("[%s][%d] World name=%s"), TEXT(__FUNCTION__), __LINE__, *World->GetName());
	LogCurrentState();
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLRawDataLogger::Tick(float DeltaTime)
{
	//UE_LOG(LogTemp, Error, TEXT("[%s][%d] World name=%s"), TEXT(__FUNCTION__), __LINE__, *World->GetName());
	LogCurrentState();
}

// Return if object is ready to be ticked
bool USLRawDataLogger::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLRawDataLogger::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLRawDataLogger, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */

// Log initial state of the world (static and dynamic entities)
void USLRawDataLogger::LogInitialState()
{	
	// Create async worker with the raw data objects
	RawDataLogWorker = new FAsyncTask<SLRawDataAsyncWorker>(GetWorld());

	// Log all semantically annotated objects 
	RawDataLogWorker->GetTask().SetAllSemanticallyLoggedObjects();

	// Start async worker
	RawDataLogWorker->StartBackgroundTask();
	
	// Wait for worker to complete (we only use the blocking wait for the initial state log)
	RawDataLogWorker->EnsureCompletion();

	// Remove all non-dynamic objects from worker
	RawDataLogWorker->GetTask().RemoveAllNonDynamicObjects();
}

// Log current state of the world (dynamic objects that moved more than the distance threshold)
void USLRawDataLogger::LogCurrentState()
{
	// Start async worker
	if (RawDataLogWorker->IsDone())
	{
		RawDataLogWorker->StartBackgroundTask();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] SKIP new task "), TEXT(__FUNCTION__), __LINE__);
	}
}
