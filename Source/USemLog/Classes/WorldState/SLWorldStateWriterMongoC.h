// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "ISLWorldStateWriter.h"
#if SL_WITH_LIBMONGO_C
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


/**
 * Raw data writer to mongo 
 */
class FSLWorldStateWriterMongoC : public ISLWorldStateWriter
{
public:
	// Default constr
	FSLWorldStateWriterMongoC();

	// Init constr
	FSLWorldStateWriterMongoC(const FSLWorldStateWriterParams& InParams);

	// Destr
	virtual ~FSLWorldStateWriterMongoC();

	// Init
	virtual void Init(const FSLWorldStateWriterParams& InParams) override;

	// Finish
	virtual void Finish() override;

	// Write the data
	virtual void Write(float Timestamp,
		TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		FSLGazeData& GazeData,
		bool bCheckAndRemoveInvalidEntities = true) override;

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);

	// Create indexes on the logged data, usually called after logging
	bool CreateIndexes() const;

#if SL_WITH_LIBMONGO_C
	// Add non skeletal actors to array
	void AddActorEntities(TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		bson_t* out_doc, uint32_t& idx);

	// Add non skeletal components to array
	void AddComponentEntities(TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		bson_t* out_doc, uint32_t& idx);

	// Add skeletal actors to array
	void AddSkeletalEntities(TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		bson_t* out_doc, uint32_t& idx);

	// Add gaze data
	void AddGazeData(const FSLGazeData& GazeData, bson_t* out_doc);

	// Add skeletal bones to array
	void AddSkeletalBones(USkeletalMeshComponent* SkelComp, bson_t* out_doc);

	// Add pose to document
	void AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* out_doc);

private:
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
