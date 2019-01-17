// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongo.h"
#if SLVIS_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <string.h>
#include <iostream>
THIRD_PARTY_INCLUDES_END
using bsoncxx::builder::basic::kvp;
#endif //SLVIS_WITH_LIBMONGO

// Ctor
USLVisImageWriterMongo::USLVisImageWriterMongo()
{
	bIsInit = false;
}

// Dtor
USLVisImageWriterMongo::~USLVisImageWriterMongo()
{
}

// Init
void USLVisImageWriterMongo::Init(const FSLVisImageWriterParams& InParams)
{
	bIsInit = USLVisImageWriterMongo::Connect(InParams.Location, InParams.EpisodeId, InParams.ServerIp, InParams.ServerPort);
}

// Write data
void USLVisImageWriterMongo::Write(const TArray<uint8>& InCompressedBitmap,
	const FSLVisImageMetadata& Metadata)
{
#if SLVIS_WITH_LIBMONGO
	// Create a bson document and array to store the entities
	bsoncxx::builder::basic::document bson_doc{};
	bson_doc.append(kvp("timestamp", bsoncxx::types::b_double{ Metadata.Timestamp }));
	//bson_doc.append(kvp("data", bsoncxx::types::b_binary{ InCompressedBitmap }));
	try
	{
		mongo_coll.insert_one(bson_doc.view());
	}
	catch (const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
	}
#endif //SLVIS_WITH_LIBMONGO
}

bool USLVisImageWriterMongo::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Params: DBName=%s; Collection=%s; IP=%s; Port=%d;"),
		TEXT(__FUNCTION__), __LINE__, *DBName, *EpisodeId, *ServerIp, ServerPort);

#if SLVIS_WITH_LIBMONGO
	try
	{
		// Get current mongo instance, or create a new one (static variable)
		mongocxx::instance::current();

		// Client connection options
		mongocxx::options::client client_options;
		mongocxx::options::ssl ssl_options;

		// If the server certificate is not signed by a well-known CA,
		// you can set a custom CA file with the `ca_file` option.
		// ssl_options.ca_file("/path/to/custom/cert.pem");

		// If you want to disable certificate verification, you
		// can set the `allow_invalid_certificates` option.
		// ssl_options.allow_invalid_certificates(true);

		client_options.ssl_opts(ssl_options);

		// Create the connection URI
		//FString Uri = TEXT("mongodb://") + IP + TEXT(":") + FString::FromInt(Port);

		// Create the mongo client/connection
		// drivers are designed to succeed during client construction,
		// regardless of the state of servers
		// i.e. can return true even if it is no connected to the server
		mongo_conn = mongocxx::client{ mongocxx::uri {} };
		//mongo_conn = mongocxx::client{ mongocxx::uri {}, client_options };
		//mongo_conn = mongocxx::client { mongocxx::uri {TCHAR_TO_UTF8(*Uri)}, client_options};
		//mongo_conn = mongocxx::client{ mongocxx::uri{"mongodb://127.0.0.1"}, client_options };
		if (mongo_conn)
		{
			//FString MongoUri = FString(mongo_conn.uri().to_string().c_str());
			//UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo client with URI: %s "),
			//	TEXT(__FUNCTION__), __LINE__, *MongoUri);
		}
		else
		{
			// Something went wrong
			return false;
		}

		// Dummy call to make sure that the server is online
		mongo_conn.list_databases();

		// Set/create the mongo database and collection
		mongo_db = mongo_conn[TCHAR_TO_UTF8(*DBName)];
		mongo_coll = mongo_db[TCHAR_TO_UTF8(*EpisodeId)];
	}
	catch (const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		return false;
	}
	return true;
#else
	return false;
#endif //SLVIS_WITH_LIBMONGO
}