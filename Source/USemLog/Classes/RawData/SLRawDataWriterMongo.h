// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLRawDataWriter.h"
#if USE_LIBMONGO
#include "mongoc.h"
#endif //USE_LIBMONGO

// Forward declaration
class FSLRawDataAsyncWorker;

/**
 * Raw data logger to a mongo database
 */
class FSLRawDataWriterMongo : public ISLRawDataWriter
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
		uint16 MongoPort=27017);
#if USE_LIBMONGO
	// Add actors
	void AddActors(bson_t& OutBsonEntitiesArr);

	// Add components
	void AddComponents(bson_t& OutBsonEntitiesArr);

	// Get entry as Bson object
	bson_t GetAsBsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Write entry to db
	void WriteToMongo(bson_t*& InRootObj, mongoc_collection_t* &collection);
#endif //USE_LIBMONGO
	// Pointer to worker parent (access to raw data structure)
	FSLRawDataAsyncWorker* WorkerParent;

	bool bConnect;
#if USE_LIBMONGO
	// Pointer to monge database
	mongoc_client_t *client;
	mongoc_database_t *database;
	mongoc_collection_t *collection;
#endif //USE_LIBMONGO
};
