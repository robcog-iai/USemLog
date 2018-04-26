// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLRawDataAsyncWorker.h"
#include "SLRawDataLogger.generated.h"

/**
 * Raw (subsymbolic) data logger
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

	// Log data to json file
	void LogToJson(bool bInLogToJson);

	// Log data to bson file
	void LogToBson(bool bInLogToBson);

	// Log data to mongodb
	void LogToMongo(bool bInLogToMongo, const FString& InMongoIP, uint16 MongoPort);

	// Start logger
	void Start(UWorld* InWorld, const FString& LogDirectory, const FString& InEpisodeId, const float UpdateRate, const float DistanceThreshold);

private:
	// Set update rate by binding to tick or a custom update rate using a timer callback
	void SetLoggerUpdateRate(const float UpdateRate);

	// Set the file handle for the logger
	void SetLoggerFileHandle(const FString& LogDirectory, const FString& InEpisodeId);

	// Timer callback (timer tick)
	void TimerCallback();

	/** Begin FTickableGameObject interface */
	// Called after ticking all actors, DeltaTime is the time passed since the last call.
	virtual void Tick(float DeltaTime) override;

	// Return if object is ready to be ticked
	virtual bool IsTickable() const override;

	// Return the stat id to use for this tickable
	virtual TStatId GetStatId() const override;
	/** End FTickableGameObject interface */

	// Log initial state of the world (static and dynamic entities)
	void LogInitialState();

	// Log current state of the world (dynamic objects that moved more than the distance threshold)
	void LogCurrentState();

	// Raw data async worker
	FAsyncTask<SLRawDataAsyncWorker>* RawDataLogWorker;

	// File handle to write the raw data to file
	IFileHandle* FileHandle;

	// Pointer to world
	UWorld* World;

	// True if the object can be ticked
	bool bIsTickable;

	// Squared distance threshold for logging movable objects
	float DistanceThresholdSquared;

	// Log data to json file
	bool bLogToJson;

	// Log data to bson file
	bool bLogToBson;

	// Log data to mongodb
	bool bLogToMongo;
};
