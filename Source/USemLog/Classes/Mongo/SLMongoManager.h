// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
#endif // SL_WITH_LIBMONGO_C
#include "SLMongoManager.generated.h"

/*
* Skeletal actor and its bones poses
*/
struct FSLMongoSkeletalPose
{
	// Pose of the actor
	FTransform SkeletalActorPose;

	// Map of bone names to poses
	TMap<FString, FTransform> BonePoses;
};

/*
* Structure holding the world frame at a given timestamp
*/
struct FSLMongoWorldStateFrame
{
	// Frame timestamp
	float Timestamp;

	// The pose of all the entities (id to pose)
	TMap<FString, FTransform> EntityPoses;

	// The poses of all the skeletal actors and their bones
	TMap<FString, TPair<FTransform, TMap<FString, FTransform>>> SkeletalPoses;
};

/**
*
**/
UCLASS()
class USEMLOG_API ASLMongoManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLMongoManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Connect to the mongo server
	bool ConnectToServer(const FString& Host, uint16 Port, bool bWithCheck = true);

	// Connect to a different database
	bool SetDatabase(const FString& InDBName, bool bWithCheck = true);

	// Connect to a different collection
	bool SetCollection(const FString& InCollectionName, bool bWithCheck = true);

	// Connect to a different database and collection
	bool SetDatabaseAndCollection(const FString& InDBName, const FString& InCollectionName, bool bWithCheck = true)
	{
		return SetDatabase(InDBName, bWithCheck) && SetCollection(InCollectionName, bWithCheck);
	}

	// Clear and disconnect from the database
	void Disconnect();

	// True if it is connected to the db
	bool IsReady() const { return bConnectedToServer && bDatabaseSet && bCollectionSet; }


	/* Entity queries*/
	// Get the entity pose at the given timestamp
	bool GetEntityPoseAt(const FString& Id, float Timestamp,
		FTransform& OutTransform) const;
	
	// Get the entity trajectory
	bool GetEntityTrajectory(const FString& Id, float StartTime, float EndTime,
		TArray<FTransform>& OutTransforms, float DeltaT = -1.f) const;


	/* Bone queries */
	// Get the bone pose at the given timestamp
	bool GetBonePoseAt(const FString& Id, const FString& BoneName, float Timestamp,
		FTransform& OutTransform) const;

	// Get the bone trajectory
	bool GetBoneTrajectory(const FString& Id, const FString& BoneName, float StartTime, float EndTime,
		TArray<FTransform>& OutTransforms, float DeltaT = -1.f) const;


	/* Skeletal queries */
	// Get the skeletal pose at the given timestamp
	bool GetSkeletalPoseAt(const FString& Id, float Timestamp,
		TPair<FTransform, TMap<FString, FTransform>>& OutSkeletalPose) const;

	// Get the skeltal trajectory
	bool GetSkeletalTrajectory(const FString& Id, float StartTime, float EndTime,
		TArray<TPair<FTransform, TMap<FString, FTransform>>>& OutSkeletalPoses, float DeltaT = -1.f) const;


	/* World queries */
	// Get the state of all the entities in the world at a given time
	bool GetWorldStateAt(float Timestamp, TMap<FString, FTransform>& OutEntityPoses,
		TMap<FString, TPair<FTransform, TMap<FString, FTransform>>>& OutSkeletalPoses) const;

	// Get the state of all the entities in the world between the timestamps
	bool GetAllWorldStates(TArray<FSLMongoWorldStateFrame>& OutWorldStates) const;

	/* Gaze queries */
	// Get the target and the origin pose at the timestamp
	bool GetGazePose(float Timestamp,
		FVector& OutTarget, FVector& OutOrigin) const;

	// Get the gaze poses between the timestamps
	bool GetGazeTrajectory(float StartTime, float EndTime,
		TArray<FVector>& OutTarget, TArray<FVector>& OutOrigin, float DeltaT = -1.f) const;
	
private:
	/* Helpers */
	// Get the unique identifiers of the world entities
	bool GetAllIdsInTheWorld(TArray<FString>& OutEntityIds, TArray<FString>& OutSkeletalIds) const;

#if SL_WITH_LIBMONGO_C
	// Get transform from doc with "loc" and "rot" fields
	FTransform GetPose(const bson_t* doc) const;

	// Get the timestamp value from "timestamp" field
	double GetTs(const bson_t* doc) const;

	// Get transform from iter with "loc" and "rot" fields
	FTransform GetPose(const bson_iter_t* iter) const;
#endif // SL_WITH_LIBMONGO_C
	
private:
	// Connected to server
	bool bConnectedToServer;

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
