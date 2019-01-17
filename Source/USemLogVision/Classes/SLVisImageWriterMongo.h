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
#include "SLVisImageWriterMongo.generated.h"

/**
 * 
 */
UCLASS()
class USLVisImageWriterMongo : public UObject, public ISLVisImageWriterInterface
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLVisImageWriterMongo();

	// Dtor
	~USLVisImageWriterMongo();

	// Init
	virtual void Init(const FSLVisImageWriterParams& InParams) override;

	// Write data
	virtual void Write(const TArray<uint8>& InCompressedBitmap, 
		const FSLVisImageMetadata& Metadata) override;

private:
	// Connect to the database
	bool Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort);

private:	
#if SLVIS_WITH_LIBMONGO
	// Must be created before using the driver and must remain alive for as long as the driver is in use
	//mongocxx::instance mongo_inst;

	// Mongo connection client
	mongocxx::client mongo_conn;

	// Database to access
	mongocxx::database mongo_db;

	// Database collection
	mongocxx::collection mongo_coll;
#endif //SLVIS_WITH_LIBMONGO
};
