// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Async/AsyncWork.h"
#include "IRawDataWriter.h"

/**
* Raw data structure for the logged entities
*/
template <typename T> 
struct TSLRawDataEntity
{
	// Default constructor
	TSLRawDataEntity() {};

	// Constructor with init
	TSLRawDataEntity(TWeakObjectPtr<T> InEntity,
		const FString& InId,
		const FString& InClass,
		FVector InPrevLoc = FVector(BIG_NUMBER)) :
		Entity(InEntity),
		Id(InId),
		Class(InClass),
		PrevLoc(InPrevLoc)
	{};

	// Actor weak pointer
	TWeakObjectPtr<T> Entity;

	// Unique id of the actor
	FString Id;

	// Class of the actor
	FString Class;

	// Previous location 
	FVector PrevLoc;
};


/**
 * Async worker to log raw data
 */
class FSLRawDataAsyncWorker : public FNonAbandonableTask
{
	// Needed if DoWork() needs access private data from this class
	friend class FAsyncTask<FSLRawDataAsyncWorker>;
	//friend class FAutoDeleteAsyncTask<SLRawDataAsyncWorker>;
	
	// Writer needs access to the private data of this class (world, data arrays etc.)
	friend class FSLRawDataWriterJson;
	friend class FSLRawDataWriterBson;
	friend class FSLRawDataWriterMongo; 


public:
	// Constructor
	FSLRawDataAsyncWorker();

	// Destructor
	~FSLRawDataAsyncWorker();

	// Init worker, load models to log from world
	void Init(UWorld* InWorld, const float DistanceThreshold);

	// Log data to json file
	void SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId);

	// Log data to bson file
	void SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId);

	// Log data to mongodb
	void SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort);
	
	// Remove all non-dynamic objects from array
	void RemoveAllNonDynamicObjects();

private:
	// FAsyncTask - async work done here
	void DoWork();

	// Needed by unreal internally
	FORCEINLINE TStatId GetStatId() const;

	// Array of semantically annotated actors to be logged
	TArray<TSLRawDataEntity<AActor>> RawDataActors;

	// Array of semantically annotated components to be logged
	TArray<TSLRawDataEntity<USceneComponent>> RawDataComponents;

	// Distance squared threshold
	float DistanceSquaredThreshold;

	// Pointer to world
	UWorld* World;

	// Raw data writer
	TSharedPtr<IRawDataWriter> Writer;
};
