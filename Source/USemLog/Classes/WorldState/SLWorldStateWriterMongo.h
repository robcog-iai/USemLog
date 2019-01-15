// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLWorldStateWriterInterface.h"
#if WITH_LIBMONGO
#include <mongocxx/client.hpp>
#endif //WITH_LIBMONGO
#include "SLWorldStateWriterMongo.generated.h"

// Forward declaration
class FSLWorldStateAsyncWorker;

/**
 * Raw data logger to mongo database
 */
UCLASS()
class USLWorldStateWriterMongo : public UObject, public ISLWorldStateWriterInterface
{
	GENERATED_BODY()

public:
	// Default constr
	USLWorldStateWriterMongo();

	// Destr
	virtual ~USLWorldStateWriterMongo();

	// Init
	virtual void Init(const FSLWorldStateWriterParams& InParams) override;

	// Write the data
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) override;

	// Create indexes from the logged data, usually called after logging
	bool CreateIndexes();

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& IP, uint16 Port);
	
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
