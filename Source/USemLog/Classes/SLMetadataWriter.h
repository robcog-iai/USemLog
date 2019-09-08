// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "WorldState/ISLWorldStateWriter.h"
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
 * Writes semantic metadata 
 */
struct USEMLOG_API FSLMetadataWriter
{
public:
	// Default constructor
	FSLMetadataWriter();

	// Destructor
	~FSLMetadataWriter();

	// Init
	void Init(const FSLWorldStateWriterParams& WriterParams, const FString& InTaskDescription);

	// Write the environment metadata
	void Start();

	// Write events metadata
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);
	
	// Add the task metadata (e.g. SemLog.meta)
	void WriteTaskMetadata(const FString& DBName, const FString& TaskDescription);

	// Write the environment metadata
	void WriteEnvironmentMetadata();

	// Write the episode events metadata
	void WriteEventsMetadata();

	// Create databased for faster lookups
	bool CreateIndexes() const;

#if SL_WITH_LIBMONGO_C
	// Add entities data to the bson document
	void AddEntities(bson_t* out_doc);

	// Add camera views
	void AddCameraViews(bson_t* out_doc);

	// Add semantic events data to the bson document
	void AddEvents(bson_t* out_doc);

	// Add pose to document
	void AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* out_doc);
#endif //SL_WITH_LIBMONGO_C

private:
	// Set when initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

#if SL_WITH_LIBMONGO_C
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;

	// Insertion document id
	bson_oid_t oid;
#endif //SL_WITH_LIBMONGO_C
};