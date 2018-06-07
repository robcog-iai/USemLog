// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RawData/SLRawDataAsyncWorker.h"
#include "SLRawDataLogger.generated.h"

/**
 * Raw (subsymbolic) data logger
 * Inherit from FTickableGameObject to have it's own tick
 */
UCLASS()
class USEMLOG_API USLRawDataLogger : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLRawDataLogger();

	// Destructor
	~USLRawDataLogger();

	// Init Logger
	void Init(const float DistanceThreshold);

	// Start logger
	void Start(const float UpdateRate);

	// Finish logger
	void Finish();

	// Log data to json file
	void SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId);

	// Log data to bson file
	void SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId);

	// Log data to mongodb
	void SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort);

private:
	// Set update rate by binding to tick or a custom update rate using a timer callback
	void SetUpdateRate(const float UpdateRate);

	// Timer callback (timer tick)
	void TimerTick();

	/** Begin FTickableGameObject interface */
	// Called after ticking all actors, DeltaTime is the time passed since the last call.
	virtual void Tick(float DeltaTime) override;

	// Return if object is ready to be ticked
	virtual bool IsTickable() const override;

	// Return the stat id to use for this tickable
	virtual TStatId GetStatId() const override;
	/** End FTickableGameObject interface */

	// Log initial state of the world (static and dynamic entities)
	void LogInitialWorldState();

	// Log current state of the world (dynamic objects that moved more than the distance threshold)
	void LogCurrentWorldState();

	// True if the object can be ticked (used by FTickableGameObject)
	bool bIsTickable;

	// Async worker to log the raw data
	FAsyncTask<FSLRawDataAsyncWorker>* AsyncWorker;
};
