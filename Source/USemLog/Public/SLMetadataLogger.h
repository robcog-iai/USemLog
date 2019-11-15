// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "UObject/NoExportTypes.h"
#include "SLItemScanner.h"
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
#include "SLMetadataLogger.generated.h"

/**
 * Writes task and episode related metadata
 */
UCLASS()
class USEMLOG_API USLMetadataLogger : public UObject
{
	GENERATED_BODY()

	// Give access to private methods
	friend class USLItemScanner;
	
public:
	// Ctor
	USLMetadataLogger();

	// Dtor
	~USLMetadataLogger();
	
	// Init logger
	void Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort, 
		bool bOverWrite = false, bool bScanItems = false, FSLItemScanParams ScanParams = FSLItemScanParams());

	// Start logger
	void Start(const FString& InTaskDescription);

	// Finish logger
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Connect to the database, if overwrite is true, remove existing collection
	bool Connect(const FString& DBName, const FString& ServerIp, uint16 ServerPort, bool bOverwrite);

	// Disconnect and clean db connection
	void Disconnect();

	// Create indexes on the data
	void CreateIndexes();

	// Create the scan entry bson document (a scan entry contains all the scan images of a given class)
	void StartScanEntry(const FString& Class, int32 ResX, int32 ResY);

	// Add pose scan data
	void AddScanPoseEntry(const FSLScanPoseData& ScanPoseData);
	
	// Write and clear the scan entry to the database
	void FinishScanEntry();
	
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
	void AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* doc);

	// Add image bounding box to document
	void AddImgBBChild(const FIntPoint& Min, const FIntPoint& Max, bson_t* doc);
#endif //SL_WITH_LIBMONGO_C
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Helper class for scanning the items from the world
	UPROPERTY() // Avoid GC
	USLItemScanner* ItemsScanner;

#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;

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
