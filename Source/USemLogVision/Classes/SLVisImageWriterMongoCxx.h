// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisImageWriterInterface.h"
#if SLVIS_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START
#include "mongocxx/client.hpp"
THIRD_PARTY_INCLUDES_END
#endif //SLVIS_WITH_LIBMONGO
#include "SLVisImageWriterMongoCxx.generated.h"

/**
 *  * Writes image data to mongodb using the C++ driver
 */
UCLASS()
class USLVisImageWriterMongoCxx : public UObject, public ISLVisImageWriterInterface
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLVisImageWriterMongoCxx();

	// Dtor
	~USLVisImageWriterMongoCxx();

	// Init
	virtual void Init(const FSLVisImageWriterParams& InParams) override;

	// Called when done writing
	virtual void Finish() override;
	
	// Write the images at the timestamp
	virtual void Write(const FSLVisStampedData& StampedData) override;

	// Skip the current timestamp (images already inserted)
	bool ShouldSkipThisTimestamp(float Timestamp);

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);

	// Re-create the indexes
	bool CreateIndexes();

private:
	// Generate a new entry point for the images
	bool bCreateNewDocument;

#if SLVIS_WITH_LIBMONGO
	// Must be created before using the driver and must remain alive for as long as the driver is in use
	//mongocxx::instance mongo_inst;

	// Mongo connection client
	mongocxx::client mongo_conn;

	// Database to access
	mongocxx::database mongo_db;

	// Database collection
	mongocxx::collection mongo_coll;

	// Gridfs bucket to upload the image data
	mongocxx::gridfs::bucket gridfs_bucket;

	// Cached document _id for the next insertion
	bsoncxx::types::b_oid insertion_doc_id;
#endif //SLVIS_WITH_LIBMONGO
};
