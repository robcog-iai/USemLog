// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLWorldStateWriter.h"
#if WITH_LIBMONGO
#include <mongocxx/client.hpp>
#endif //WITH_LIBMONGO

// Forward declaration
class FSLWorldStateAsyncWorker;

/**
 * Raw data logger to mongo database
 */
class FSLWorldStateWriterMongo : public ISLWorldStateWriter
{
public:
	// Default constr
	FSLWorldStateWriterMongo(float DistanceStepSize, float RotationStepSize, 
		const FString& Location, const FString& EpisodeId, const FString& HostIP, uint16 HostPort);

	// Destr
	virtual ~FSLWorldStateWriterMongo();

	// Write the data
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) override;

private:
	// Connect to the database
	bool Connect(const FString& DB, const FString& EpisodeId, const FString& IP, uint16 Port = 27017);
	
#if WITH_LIBMONGO
	// Get non skeletal actors as bson array
	void AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		bsoncxx::builder::basic::array& out_bson_arr);

	// Get skeletal actors as bson array
	void AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		bsoncxx::builder::basic::array& out_bson_arr);

	// Get non skeletal components as bson array
	void AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		bsoncxx::builder::basic::array& out_bson_arr);

	// Add the pose information of the entity
	void GetPoseAsBsonEntry(const FVector& InLoc, const FQuat& InQuat, 
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
#endif //WITH_LIBMONGO
};
