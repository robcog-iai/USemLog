// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

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

/**
 * 
 */
class FSLMongoQueryDBHandler
{
public:
	// Ctor
	FSLMongoQueryDBHandler();

	// Dtor
	~FSLMongoQueryDBHandler();

	// Connect to the server
	bool Connect(const FString& ServerIp, uint16 ServerPort);

	// Set database
	bool SetDatabase(const FString& InDBName);

	// Set collection
	bool SetCollection(const FString& InCollName);

	// Disconnect and clean db connection
	void Disconnect();

	// Everything is set in order to query the data
	bool IsReady() const { return bConnected && bDatabaseSet && bCollectionSet; };

	/* Queries */
	// Get the pose of the individual at the given time
	FTransform GetIndividualPoseAt(const FString& Id, float Ts) const;

	// Get the poses of the individual between the given timestamps
	TArray<FTransform> GetIndividualTrajectory(const FString& Id, float StartTs, float EndTs, float DeltaT = -1.f) const;

	// Get skeletal individual pose
	TPair<FTransform, TMap<int32, FTransform>> GetSkeletalIndividualPoseAt(const FString& Id, float Ts) const;

	// Get skeletal individual trajectory
	TArray<TPair<FTransform, TMap<int32, FTransform>>> GetSkeletalIndividualTrajectory(const FString& Id, float StartTs, float EndTs, float DeltaT = -1.f) const;

	// Get the whole episode data
	TArray<TPair<float, TMap<FString, FTransform>>> GetEpisodeData() const;

	// Get the whole episode data in an async thread
	TArray<TPair<float, TMap<FString, FTransform>>> GetEpisodeDataAsync() const;

	// Get the episode data at the given timestamp (frame)
	TMap<FString, FTransform> GetFrameData(float Ts);

private:
#if SL_WITH_LIBMONGO_C
	/* Helpers */
	// Get the pose data from bson document
	FTransform GetPose(const bson_t* doc) const;

	// Get the pose data from bson iterator
	FTransform GetPose(const bson_iter_t* iter) const;

	// Get the timestamp value from document (used for trajectory delta time comparison)
	double GetTs(const bson_t* doc) const;
#endif // SL_WITH_LIBMONGO_C

private:
	// Connected to server
	bool bConnected;

	// Connected to a database
	bool bDatabaseSet;

	// Connected to a database
	bool bCollectionSet;

#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// World state data collection
	mongoc_collection_t* collection;

	// Entity ids meta data collection
	mongoc_collection_t* meta_collection;
#endif // SL_WITH_LIBMONGO_C
};
