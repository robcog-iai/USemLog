// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
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
		bool bCheckAndRemoveInvalidEntities = true) override;

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);

	// Create indexes from the logged data, usually called after logging
	bool CreateIndexes();

#if SL_WITH_LIBMONGO_C
//	// Get non skeletal actors as bson array
//	void AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
//		bsoncxx::builder::basic::array& out_bson_arr);
//
//	// Get skeletal actors as bson array
//	void AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
//		bsoncxx::builder::basic::array& out_bson_arr);
//
//	// Get non skeletal components as bson array
//	void AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
//		bsoncxx::builder::basic::array& out_bson_arr);
//
//	// Add the pose information of the document
//	void AddPoseToDocument(const FVector& InLoc, const FQuat& InQuat,
//		bsoncxx::builder::basic::document& out_doc);
//
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
