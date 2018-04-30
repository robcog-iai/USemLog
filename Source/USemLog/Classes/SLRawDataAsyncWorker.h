// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Async/AsyncWork.h"
#include "Tags.h"

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
	// Needed in order for DoWork() to access to private data from this class
	friend class FAsyncTask<FSLRawDataAsyncWorker>;
	//friend class FAutoDeleteAsyncTask<SLRawDataAsyncWorker>;

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
	// Set file handle
	void SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId, const FString& InFileExt);
	
	//// Set mongo connection
	//void SetMongoConnection(const FString& LogDirectory, const FString& InEpisodeId, const FString& InFileExt);
	
	// FAsyncTask - async work done here
	void DoWork();

	/* Begin LogTo_ function declarations */
	// Function pointer type declaration
	typedef void(FSLRawDataAsyncWorker::*LogToFunctionPointerType)();

	// Function pointer definition
	LogToFunctionPointerType LogToFunctionPointer;

	// Log to nothing
	void LogTo_Default();

	// Log to json
	void LogTo_Json();

	// Log to bson
	void LogTo_Bson();

	// Log to mongo
	void LogTo_Mongo();
	/* End LogTo_ function declarations */

	// Get entry as json object
	TSharedPtr<FJsonObject> GetAsJsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

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

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
