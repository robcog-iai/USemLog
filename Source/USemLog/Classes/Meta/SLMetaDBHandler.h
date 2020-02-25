// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Meta/SLMetaScannerStructs.h"

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
class FSLMetaDBHandler
{
public:
	// Ctor
	FSLMetaDBHandler();

	// Connect to the database
	bool Connect(const FString& DBName, const FString& ServerIp, uint16 ServerPort, bool bRemovePrevEntries, bool bScanItems);

	// Disconnect and clean db connection
	void Disconnect() const;

	// Create indexes on the inserted data
	void CreateIndexes() const;

	// Write the first metadata entry
	void WriteFirstDocument(const FString& InTaskDescription);

	// Create the scan entry bson document (a scan entry contains all the scan images of a given class)
	void StartScanEntry(const FString& Class, int32 ResX, int32 ResY);

	// Add pose scan data
	void AddScanPoseEntry(const FSLScanPoseData& ScanPoseData);

	// Write and clear the scan entry to the database
	void FinishScanEntry();

private:
#if SL_WITH_LIBMONGO_C
	// Add image to gridfs, output the oid
	void AddToGridFs(const TArray<uint8>& CompressedBitmap, bson_oid_t* out_oid);

	// Write the task description
	void AddTaskDescription(const FString& InTaskDescription, bson_t* doc);

	// Add the environment data (skeletal and non-skeletal entities)
	void AddEnvironmentData(bson_t* doc);

	// Add the existing camera views
	void AddCameraViews(bson_t* doc);

	// Add pose to document
	void AddPoseDoc(const FVector& InLoc, const FQuat& InQuat, bson_t* doc);

	// Add image bounding box to document
	void AddBBDoc(const FIntPoint& Min, const FIntPoint& Max, bson_t* doc);
#endif //SL_WITH_LIBMONGO_C

private:
	// Total number of pixels in the current image
	int64 TotalNumPixels;

#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;

	// Collection for storing the scans
	mongoc_collection_t* scans_collection;

	// Insert scans binaries
	mongoc_gridfs_t* gridfs;

	// Scan entry doc (holds all the scanned images of the given class)
	bson_t* scan_entry_doc;

	// Scan pose array (holds the array of image types and the pose of the camera)
	bson_t* scan_pose_arr;

	// Image scan array doc index
	uint32_t scan_pose_arr_idx;
#endif //SL_WITH_LIBMONGO_C
};
