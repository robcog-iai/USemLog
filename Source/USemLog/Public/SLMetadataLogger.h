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

public:
	// Ctor
	USLMetadataLogger();

	// Dtor
	~USLMetadataLogger();
	
	// Init logger
	void Init(const FString& InTaskId, const FString InServerIp, uint16 InServerPort,
		UWorld* InWorld, bool bScanItems, FIntPoint Resolution, bool bScanViewModeUnlit, bool bIncludeScansLocally,  bool bOverwrite = false);

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

	// Create meta document
	void CreateDoc();

	// Write the task description
	void AddTaskDescription(const FString& InTaskDescription);

	// Add the environment data (skeletal and non-skeletal entities)
	void AddEnvironmentData();

	// Add the existing camera views
	void AddCameraViews();

	// Add item image scans
	void AddScans();
	
	// Insert the document to the collection
	void InsertDoc();

#if SL_WITH_LIBMONGO_C
	// Add pose to document
	void AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* out_doc);
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

	// Document id, the whole collection will consist of one document
	bson_t* doc;
#endif //SL_WITH_LIBMONGO_C
};
