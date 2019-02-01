// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisImageWriterInterface.h"
#if SLVIS_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START
#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include <mongoc/mongoc.h>
	#include "Windows/HideWindowsPlatformTypes.h"
#else
	#include <mongoc/mongoc.h>
#endif // #if PLATFORM_WINDOWS
THIRD_PARTY_INCLUDES_END
#endif //SLVIS_WITH_LIBMONGO
#include "SLVisImageWriterMongoC.generated.h"


/**
* Parameters for creating an image data writer
* data may be stale
*/
struct FSLVisWorldStateEntryParams
{
#if SLVIS_WITH_LIBMONGO
	// If true, we know for sure there is no stale data
	bool bAllDataIsValid;

	// Entry timestamp
	float Timestamp;

	// Distance from query ts to entry ts
	float TimeDistance;

	// Contains images
	bool bContainsImageData;

	// _id of the entry as string
	char oid_str[25];

	// Default constructor
	FSLVisWorldStateEntryParams()
	{
		bAllDataIsValid = false;
		Timestamp = -1.f; // Negative represents stale data
		TimeDistance = -1.f;
		bContainsImageData = false;
	};

	// Return result as string
	FString ToString() const 
	{
		//char oidstr[25];
		//bson_oid_to_string(oid, oidstr);
		return FString::Printf(TEXT("bAllDataIsValid:[%s] Timestamp:[%f] TimeDistance:[%f] bContainsImageData:[%s] oid[%s]"),
			bAllDataIsValid ? TEXT("true") : TEXT("false"),
			Timestamp,
			TimeDistance,
			bContainsImageData ? TEXT("true") : TEXT("false"),
			*FString(oid_str)
		);
	}
#endif //SLVIS_WITH_LIBMONGO
};

/**
 * Writes image data to mongodb using the C driver
 */
	UCLASS()
	class USLVisImageWriterMongoC : public UObject, public ISLVisImageWriterInterface
{
	GENERATED_BODY()

public:
	// Ctor
	USLVisImageWriterMongoC();

	// Dtor
	~USLVisImageWriterMongoC();

	// Init
	virtual void Init(const FSLVisImageWriterParams& InParams) override;

	// Called when done writing
	virtual void Finish() override;

	// Write the images at the timestamp
	virtual void Write(float Timestamp, const TArray<FSLVisImageData>& ImagesData) override;

	// Skip the current timestamp (images already inserted)
	bool ShouldSkipThisTimestamp(float Timestamp);

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);

	// Get parameters about the closest entry to the given timestamp
	void GetWorldStateParamsAt(float InTimestamp, bool bSearchBeforeTimestamp, FSLVisWorldStateEntryParams& OutParams);

	// Re-create the indexes (there could be new entries)
	bool CreateIndexes();

#if SLVIS_WITH_LIBMONGO
	// Write image data to gridfs, out param the oid of the file/entry, return true on success
	bool SaveToGridFS(const FSLVisImageData& ImgData, bson_oid_t* out_oid);
#endif //SLVIS_WITH_LIBMONGO

private:
	// Generate a new entry point for the images
	bool bCreateNewEntry;

	// Min time offset for a new db entry
	float TimeRange;

#if SLVIS_WITH_LIBMONGO
	// Server uri
	mongoc_uri_t* uri;

	// MongoC connection client
	mongoc_client_t* client;

	// Database to access
	mongoc_database_t* database;

	// Database collection
	mongoc_collection_t* collection;

	// Gridfs handle to insert large binary data to the db
	mongoc_gridfs_t *gridfs;
	//mongoc_gridfs_bucket_t *bucket; // available starting 1.14

	// _id of the object (world state) where to insert the images
	char* ws_oid_str;
#endif //SLVIS_WITH_LIBMONGO
};
