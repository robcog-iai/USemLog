// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Vision/SLVisionStructs.h"
#include "Animation/SkeletalMeshActor.h"

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
 * Helper class for reading and writing vision related data to mongodb
 */
class FSLVisionDBHandler
{
public:
	// Ctor
	FSLVisionDBHandler();

	// Connect to the database
	bool Connect(const FString& DBName, const FString& CollName, const FString& ServerIp,
		uint16 ServerPort, bool bRemovePrevEntries);

	// Disconnect and clean db connection
	void Disconnect() const;

	// Create indexes on the inserted data
	void CreateIndexes() const;

	// Get episode data from the database (UpdateRate = 0 means all the data)
	bool GetEpisodeData(float UpdateRate, const TMap<ASkeletalMeshActor*,
		ASLVisionPoseableMeshActor*>& InSkelToPoseableMap,
		FSLVisionEpisode& OutEpisode);

	// Write current frame
	void WriteFrame(const FSLVisionFrameData& Frame) const;

private:
	// Remove any previously added vision data from the database
	void DropPreviousEntriesFromWorldColl_Legacy(const FString& DBName, const FString& CollName) const;

	// Remove any previously added vision data from the database
	void DropPreviousEntries(const FString& DBName, const FString& CollName) const;

#if SL_WITH_LIBMONGO_C
	// Helper function to get the entities data out of the bson iterator, returns false if there are no entities
	bool GetEntitiesData(bson_iter_t* doc,
		TMap<AStaticMeshActor*, FTransform>& OutEntityPoses,
		TMap<ASLVirtualCameraView*, FTransform>& OutVirtualCameraPoses) const;

	// Helper function to get the entities data out of the bson iterator, returns false if there are no entities
	bool GetSkeletalEntitiesData(bson_iter_t* doc,
		const TMap<ASkeletalMeshActor*, ASLVisionPoseableMeshActor*>& InSkelToPoseableMap,
		TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>>& OutSkeletalPoses) const;

	// Save image to gridfs, get the file oid and return true if succeeded
	bool AddToGridFs(const TArray<uint8>& InData, bson_oid_t* out_oid) const;

	// Write the bson doc containing the vision data to the entry corresponding to the timestamp
	bool WriteToWorldColl_Legacy(bson_t* doc, float Timestamp) const;

	// Write the bson doc containing the vision data to the entry corresponding to the timestamp
	bool WriteToVisionColl(bson_t* doc) const;

	// Add image bounding box to document
	void AddBBObj(const FIntPoint& Min, const FIntPoint& Max, bson_t* doc) const;
#endif //SL_WITH_LIBMONGO_C

private:
#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;

	// Vision collection
	mongoc_collection_t* vis_collection;

	// Store image binaries
	mongoc_gridfs_t* gridfs;
#endif //SL_WITH_LIBMONGO_C	
};
