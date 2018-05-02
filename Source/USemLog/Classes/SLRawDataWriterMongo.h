// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLRawDataWriter.h"

// Forward declaration
class FSLRawDataAsyncWorker;

/**
 * 
 */
class FSLRawDataWriterMongo : public FSLRawDataWriter
{
public:
	// Default constr
	FSLRawDataWriterMongo();

	// Constructor with init
	FSLRawDataWriterMongo(FSLRawDataAsyncWorker* InWorkerParent,
		const FString& InLogDB,
		const FString& InEpisodeId,
		const FString& InMongoIP,
		uint16 MongoPort);

	// Destr
	virtual ~FSLRawDataWriterMongo();

	// Init
	void Init(FSLRawDataAsyncWorker* InWorkerParent,
		const FString& InLogDB,
		const FString& InEpisodeId,
		const FString& InMongoIP,
		uint16 MongoPort);

	// Called to write the data
	virtual void WriteData() override;

private:
	// Connect to mongo db
	bool ConnectToMongo(const FString& InLogDB,
		const FString& InEpisodeId,
		const FString& InMongoIP,
		uint16 MongoPort);

	// Add actors
	void AddActors(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr);

	// Add components
	void AddComponents(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr);

	// Get entry as Bson object
	TSharedPtr<FJsonObject> GetAsBsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Write entry to db
	void WriteToMongo(const TSharedPtr<FJsonObject>& InRootObj);

	// Pointer to worker parent (access to raw data structure)
	FSLRawDataAsyncWorker* WorkerParent;
};
