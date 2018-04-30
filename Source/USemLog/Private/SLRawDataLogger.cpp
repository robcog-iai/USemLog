// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataLogger.h"
#include "Tags.h"

// Constructor
USLRawDataLogger::USLRawDataLogger()
{
}

// Destructor
USLRawDataLogger::~USLRawDataLogger()
{
	USLRawDataLogger::Stop();
}

// Init logger
void USLRawDataLogger::Init(const float InDistanceThreshold)
{
	// Create async worker to log on a separate thread
	AsyncWorker = new FAsyncTask<FSLRawDataAsyncWorker>();

	// Init async worker
	AsyncWorker->GetTask().Init(GetWorld(), InDistanceThreshold);
}

// Start logger
void USLRawDataLogger::Start(const float UpdateRate)
{
	// Log initial state of the world
	LogInitialWorldState();

	// Set logger to update on tick or on custom update rate using a timer
	SetUpdateRate(UpdateRate);
}

// Stop logger
void USLRawDataLogger::Stop()
{
	if (AsyncWorker)
	{
		// Wait for worker to complete before deleting it
		AsyncWorker->EnsureCompletion();
		delete AsyncWorker;
		AsyncWorker = nullptr;
	}
}

// Log data to json file
void USLRawDataLogger::SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	if (AsyncWorker)
	{
		AsyncWorker->GetTask().SetLogToJson(InLogDirectory, InEpisodeId);
	}
}

// Log data to bson file
void USLRawDataLogger::SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	if (AsyncWorker)
	{
		AsyncWorker->GetTask().SetLogToBson(InLogDirectory, InEpisodeId);
	}
}

// Log data to mongodb
void USLRawDataLogger::SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort)
{
	if (AsyncWorker)
	{
		AsyncWorker->GetTask().SetLogToMongo(InLogDB, InEpisodeId, InMongoIP, MongoPort);
	}
}

// Set update rate by binding to tick or a custom update rate using a timer callback
void USLRawDataLogger::SetUpdateRate(const float UpdateRate)
{
	if (UpdateRate > 0.0f)
	{
		// Update logger on custom timer tick (does not guarantees the UpdateRate value,
		// since it will be eventually triggered from the game thread tick
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLRawDataLogger::TimerTick, UpdateRate, true);
	}
	else
	{
		// Update logger on tick (updates every game thread tick, update rate can vary)
		bIsTickable = true;
	}
}

// Timer callback (timer tick)
void USLRawDataLogger::TimerTick()
{
	LogCurrentWorldState();
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLRawDataLogger::Tick(float DeltaTime)
{
	LogCurrentWorldState();
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
void USLRawDataLogger::LogInitialWorldState()
{
	// Start async worker
	AsyncWorker->StartBackgroundTask();
	
	// Wait for worker to complete (we only use the blocking wait for the initial state log)
	AsyncWorker->EnsureCompletion();

	// Remove all non-dynamic objects from worker
	AsyncWorker->GetTask().RemoveAllNonDynamicObjects();
}

// Log current state of the world (dynamic objects that moved more than the distance threshold)
void USLRawDataLogger::LogCurrentWorldState()
{
	// Start task if worker is done with its previous work
	if (AsyncWorker->IsDone())
	{
		AsyncWorker->StartBackgroundTask();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] SKIP new task "), TEXT(__FUNCTION__), __LINE__);
	}
}
