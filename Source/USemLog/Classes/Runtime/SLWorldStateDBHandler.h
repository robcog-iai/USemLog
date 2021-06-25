// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
class USLBoneIndividual;
class USLVirtualBoneIndividual;
class USLBoneConstraintIndividual;

/**
 * Async task to write to the database
 */
class FSLWorldStateDBWriterAsyncTask : public FNonAbandonableTask
{
public:
#if SL_WITH_LIBMONGO_C
	// Set the individuals
	bool Init(mongoc_collection_t* in_collection, ASLIndividualManager* Manager, float PoseTolerance, bool bInWriteSparse);
#endif //SL_WITH_LIBMONGO_C	

	// Do the db writing here
	void DoWork();

	// Needed internally
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FAnalyzeMaterialTreeAsyncTask, STATGROUP_ThreadPoolAsyncTasks); }

	// Set the simulation time
	void SetTimestamp(float InTs) { Timestamp = InTs; };

private:
	// First write where all the individuals are written irregardresly of their previous position
	int32 FirstWrite();

	// Write sparse (only individuals that moved)
	int32 WriteSparse();

	// Write all individuals (event if they did not move)
	int32 WriteAll();

#if SL_WITH_LIBMONGO_C
	// Add timestamp to the bson doc
	void AddTimestamp(bson_t* doc);

	// Add all individuals (return the number of individuals added)
	int32 AddAllIndividuals(bson_t* doc);

	// Add only the individuals that moved (return the number of individuals added)
	int32 AddIndividualsThatMoved(bson_t* doc);

	// Add skeletal individuals (return the number of individuals added)
	int32 AddSkeletalIndividals(bson_t* doc);

	// Add skeletal bones to the document
	void AddSkeletalBoneIndividuals(const TArray<USLBoneIndividual*>& BoneIndividuals,
		const TArray<USLVirtualBoneIndividual*>& VirtualBoneIndividuals,
		bson_t* doc);

	// Add skeletal bone constraints to the document
	void AddSkeletalConstraintIndividuals(const TArray<USLBoneConstraintIndividual*>& ConstraintIndividuals,
		bson_t* doc);

	// Add robot individuals (return the number of individuals added)
	int32 AddRobotIndividuals(bson_t* doc);

	// Add pose document
	void AddPose(FTransform Pose, bson_t* doc);

	// Write the bson doc to the collection
	bool UploadDoc(bson_t* doc);
#endif //SL_WITH_LIBMONGO_C


private:
	// DoWork function pointers
	typedef int32 (FSLWorldStateDBWriterAsyncTask::*WriteTypeFunctionPtr)();
	WriteTypeFunctionPtr WriteFunctionPtr;

	// Access to the individual manager
	ASLIndividualManager* IndividualManager;

	// The timestamp to write to the collection
	float Timestamp;

	// Pose diff tolerance
	float MinPoseDiff;

	// Write mode
	bool bWriteSparse;

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

	// Write metadata
	bool WriteMetadata(ASLIndividualManager* IndividualManager, const FString& MetaCollName, bool bOverwrite);

#if SL_WITH_LIBMONGO_C
	int32 AddIndividualsMetadata(ASLIndividualManager* IndividualManager, bson_t* doc);
#endif //SL_WITH_LIBMONGO_C	

	// Disconnect and clean db connection
	void Disconnect() const;

	// Create indexes on the inserted data
	bool CreateIndexes() const;

private:
	// True if connected to the db
	bool bIsInit;

	// Pointers are reset
	bool bIsFinished;

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
