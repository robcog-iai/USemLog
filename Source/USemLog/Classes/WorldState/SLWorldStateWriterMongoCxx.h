// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLWorldStateWriter.h"
#if SL_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START
#include <mongocxx/client.hpp>
THIRD_PARTY_INCLUDES_END
#endif //SL_WITH_LIBMONGO

// Forward declaration
class FSLWorldStateAsyncWorker;

/**
 * Raw data logger to mongo database
 */
class FSLWorldStateWriterMongoCxx : public ISLWorldStateWriter
{
public:
	// Default constr
	FSLWorldStateWriterMongoCxx();

	// Init constr
	FSLWorldStateWriterMongoCxx(const FSLWorldStateWriterParams& InParams);

	// Destr
	virtual ~FSLWorldStateWriterMongoCxx();

	// Init
	virtual void Init(const FSLWorldStateWriterParams& InParams) override;

	// Finish
	virtual void Finish() override;

	// Write the data
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) override;
private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);

	// Create indexes from the logged data, usually called after logging
	bool CreateIndexes();
	
#if SL_WITH_LIBMONGO
	// Get non skeletal actors as bson array
	void AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		bsoncxx::builder::basic::array& out_bson_arr);

	// Get skeletal actors as bson array
	void AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		bsoncxx::builder::basic::array& out_bson_arr);

	// Get non skeletal components as bson array
	void AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		bsoncxx::builder::basic::array& out_bson_arr);

	// Add the pose information of the document
	void AddPoseToDocument(const FVector& InLoc, const FQuat& InQuat, 
		bsoncxx::builder::basic::document& out_doc);

private:
	// Must be created before using the driver and must remain alive for as long as the driver is in use
	//mongocxx::instance mongo_inst;

	// Mongo connection client
	mongocxx::client mongo_conn;

	// Database to access
	mongocxx::database mongo_db;

	// Database collection
	mongocxx::collection mongo_coll;
#endif //SL_WITH_LIBMONGO
};
