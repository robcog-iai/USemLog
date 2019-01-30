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
 *
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

	// Re-create the indexes
	bool CreateIndexes();

private:
	// Generate a new entry point for the images
	bool bCreateNewDocument;

#if SLVIS_WITH_LIBMONGO
	// Server uri
	mongoc_uri_t *uri;

	// MongoC connection client
	mongoc_client_t *client;

	// Database to access
	mongoc_database_t *database;

	// Database collection
	mongoc_collection_t *collection;
#endif //SLVIS_WITH_LIBMONGO
};
