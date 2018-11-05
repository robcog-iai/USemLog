// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLWorldStateLogger.h"

// Constructor
USLWorldStateLogger::USLWorldStateLogger()
{
	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Default writer to local json file
	WriterType = ESLWorldStateWriterType::Json;
}

// Destructor
USLWorldStateLogger::~USLWorldStateLogger()
{
	if (!bIsFinished)
	{
		USLWorldStateLogger::Finish();
	}
}

// Init logger
void USLWorldStateLogger::Init(const float InDistanceThreshold)
{
	if (!bIsInit)
	{
		// Create async worker to log on a separate thread
		AsyncWorker = new FAsyncTask<FSLWorldStateAsyncWorker>();

		// Init async worker
		AsyncWorker->GetTask().Init(GetWorld(), InDistanceThreshold);

		// Flag as init
		bIsInit = true;
	}
}
void USLWorldStateLogger::Init(ESLWorldStateWriterType WriterType,
	const float DistanceStepSize,
	const FString& EpisodeId,
	const FString& Location,
	const FString& HostIp,
	const uint16 HostPort)
{
	// TODO
	// Writer should be done here and added as parameter to the AsyncWorker Init();
	// Should the writer should receive as input the array of items to pe logged?
	if (!bIsInit)
	{
		// Create async worker to do the writing on a separate thread
		AsyncWorker = new FAsyncTask<FSLWorldStateAsyncWorker>();

		// Init async worker
		AsyncWorker->GetTask().Init(GetWorld(), DistanceStepSize);

		// Flag as init
		bIsInit = true;
	}
}

// Start logger
void USLWorldStateLogger::Start(const float UpdateRate)
{
	if (!bIsStarted && bIsInit)
	{
		// Log initial state of the world
		USLWorldStateLogger::LogInitialWorldState();

		// Start updating
		if (UpdateRate > 0.0f)
		{
			// Update logger on custom timer tick (does not guarantees the UpdateRate value,
			// since it will be eventually triggered from the game thread tick
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLWorldStateLogger::Update, UpdateRate, true);
		}
		else
		{
			// Update logger on tick (updates every game thread tick, update rate can vary)
			bIsTickable = true;
		}

		// Set flags
		bIsStarted = true;
	}
}

// Finish logger
void USLWorldStateLogger::Finish()
{
	if (bIsStarted || bIsInit)
	{
		if (AsyncWorker)
		{
			// Wait for worker to complete before deleting it
			AsyncWorker->EnsureCompletion();
			delete AsyncWorker;
			AsyncWorker = nullptr;
		}
		// Stop update timer;
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}

		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Log data to json file
void USLWorldStateLogger::SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	if (AsyncWorker)
	{
		AsyncWorker->GetTask().SetLogToJson(InLogDirectory, InEpisodeId);
	}
}

// Log data to bson file
void USLWorldStateLogger::SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	if (AsyncWorker)
	{
		AsyncWorker->GetTask().SetLogToBson(InLogDirectory, InEpisodeId);
	}
}

// Log data to mongodb
void USLWorldStateLogger::SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort)
{
	if (AsyncWorker)
	{
		AsyncWorker->GetTask().SetLogToMongo(InLogDB, InEpisodeId, InMongoIP, MongoPort);
	}
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLWorldStateLogger::Tick(float DeltaTime)
{
	USLWorldStateLogger::Update();
}

// Return if object is ready to be ticked
bool USLWorldStateLogger::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLWorldStateLogger::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLWorldStateLogger, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */

// Log initial state of the world (static and dynamic entities)
void USLWorldStateLogger::LogInitialWorldState()
{
	// Start async worker
	AsyncWorker->StartBackgroundTask();
	
	// Wait for worker to complete (we only use the blocking wait for the initial state log)
	AsyncWorker->EnsureCompletion();

	// Remove all non-dynamic objects from worker
	AsyncWorker->GetTask().RemoveAllNonDynamicObjects();
}

// Log current state of the world (dynamic objects that moved more than the distance threshold)
void USLWorldStateLogger::Update()
{
	// Start task if worker is done with its previous work
	if (AsyncWorker->IsDone())
	{
		AsyncWorker->StartBackgroundTask();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] Previous task not finished, SKIPPING new task.."), TEXT(__FUNCTION__), __LINE__);
	}
}
