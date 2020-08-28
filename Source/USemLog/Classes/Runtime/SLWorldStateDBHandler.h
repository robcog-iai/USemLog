// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Runtime/SLLoggerStructs.h"
#include "Async/AsyncWork.h"

#if SL_WITH_LIBMONGO_C
class ASLVisionPoseableMeshActor;
THIRD_PARTY_INCLUDES_START
#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include <mongoc/mongoc.h>
	#include "Windows/HideWindowsPlatformTypes.h"
#else
	#include <mongoc/mongoc.h>
#endif // #if PLATFORM_WINDOWS
THIRD_PARTY_INCLUDES_END
#endif //SL_WITH_LIBMONGO_C

// Forward declarations
class ASLIndividualManager;
class USLBaseIndiviual;

/**
 * Async task to write to the database
 */
class FSLWorldStateDBWriterAsyncTask : public FNonAbandonableTask
{
public:
#if SL_WITH_LIBMONGO_C
	// Set the individuals
	bool Init(mongoc_collection_t* in_collection, ASLIndividualManager* Manager, float MinLinearDistance, float MinAngularDistance);
#endif //SL_WITH_LIBMONGO_C	

	// Do the db writing here
	void DoWork();

	// Needed internally
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FAnalyzeMaterialTreeAsyncTask, STATGROUP_ThreadPoolAsyncTasks); }

	// Set the simulation time
	void SetTimestamp(float InTs) { Timestamp = InTs; };

	// TODO
	// Remove individual (in case an object has been destroyed in the world)
	bool RemoveIndividual() {};

private:
	// First write where all the individuals are written irregardresly of their previous position
	int32 FirstWrite();

	// Normal write, where all the individuals are written if the tolerance theshold is passed
	int32 Write();


#if SL_WITH_LIBMONGO_C
	// Add timestamp to the bson doc
	void AddTimestamp(bson_t* doc);

	// Add all individuals (return the number of individuals added)
	int32 AddAllIndividuals(bson_t* doc);

	// Add only the individuals that moved (return the number of individuals added)
	int32 AddAllIndividualsThatMoved(bson_t* doc);

	// Add skeletal individuals (return the number of individuals added)
	int32 AddSkeletalIndividals(bson_t* doc);

	// Add robot individuals (return the number of individuals added)
	int32 AddRobotIndividuals(bson_t* doc);

	// Write the bson doc to the collection
	bool WriteDoc(bson_t* doc);
#endif //SL_WITH_LIBMONGO_C


private:
	// Do work function pointers
	typedef int32 (FSLWorldStateDBWriterAsyncTask::*WriteTypeFunctionPtr)();
	WriteTypeFunctionPtr WriteFunctionPtr;

	// Access to the individual manager
	ASLIndividualManager* IndividualManager;

	// The timestamp to write to the collection
	float Timestamp;

	// Min linear distance between the previous and the current state of the individuals
	float MinLinDist;

	// Min angular distance in order to log an individual
	float MinAngDist;

	// Individuals with their previous transform
	TArray<TPair<USLBaseIndividual*, FTransform>> IndividualsData;

#if SL_WITH_LIBMONGO_C
	// Database collection
	mongoc_collection_t* mongo_collection;
#endif //SL_WITH_LIBMONGO_C	
};


/**
 * Helper class for connecting and writing to the database
 */
class FSLWorldStateDBHandler
{
public:
	// Ctor
	FSLWorldStateDBHandler();

	// Dtor
	~FSLWorldStateDBHandler();

	// Connect to the db and set up the async writer
	bool Init(ASLIndividualManager* IndividualManager,
		const FSLWorldStateLoggerParams& InLoggerParameters,
		const FSLLoggerLocationParams& InLocationParameters,
		const FSLLoggerDBServerParams& InDBServerParameters);

	// Delegate first job to the async task
	void FirstWrite(float Timestamp);

	// Delegate job to the async task (true if the previous job was done)
	bool Write(float Timestamp);

	// Disconnect from db, clear task
	void Finish();

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& CollName, const FString& ServerIp,
		uint16 ServerPort, bool bOverwrite);

	// Disconnect and clean db connection
	void Disconnect() const;

	// Create indexes on the inserted data
	bool CreateIndexes() const;

private:
	// True if connected to the db
	bool bIsConnected;

	// Pointers are reset
	bool bIsCleared;

	// Call time of the previous writing task
	double PrevWriteCallTime;

	// Async writing to the database
	FAsyncTask<FSLWorldStateDBWriterAsyncTask>* DBWriterTask;

#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;
#endif //SL_WITH_LIBMONGO_C	
};
