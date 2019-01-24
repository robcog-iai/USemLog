// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongo.h"
#if SLVIS_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/gridfs/bucket.hpp>
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

// Finish
void USLVisImageWriterMongo::Finish()
{
	if (bIsInit)
	{
		// Re-create the indexes
		USLVisImageWriterMongo::CreateIndexes();
		bIsInit = false;
	}
}

// Write the images at the timestamp
void USLVisImageWriterMongo::Write(float Timestamp, const TArray<FSLVisImageData>& ImagesData)
{
#if SLVIS_WITH_LIBMONGO
	// Create a bson document and array to store the images
	bsoncxx::builder::basic::document bson_doc{};
	bsoncxx::builder::basic::array bson_arr{};

	// Add images array
	for (const auto& Img : ImagesData)
	{
		// Create new local document to store the img data
		bsoncxx::builder::basic::document bson_img_doc{};

		// Add camera label and type
		bson_img_doc.append(kvp("label", TCHAR_TO_UTF8(*Img.Metadata.Label)));
		bson_img_doc.append(kvp("type", TCHAR_TO_UTF8(*Img.Metadata.ViewType.ToString())));

		// Add image resolution
		bsoncxx::builder::basic::document bson_res_doc{};
		bson_res_doc.append(kvp("x", bsoncxx::types::b_int32{Img.Metadata.ResX}));
		bson_res_doc.append(kvp("y", bsoncxx::types::b_int32{Img.Metadata.ResY}));
		bson_img_doc.append(kvp("res", bson_res_doc));

		// Add binary data
		//bson_img_doc.append(kvp("data", bsoncxx::types::b_binary{
		//	bsoncxx::binary_sub_type::k_binary,
		//	static_cast<uint32_t>(Img.Data.Num()),
		//	reinterpret_cast<const uint8_t*>(Img.Data.GetData())}));

		mongocxx::options::gridfs::bucket img_bucket_options;
		//img_bucket_options.bucket_name("a_name");


		mongocxx::gridfs::bucket img_bucket = mongo_db.gridfs_bucket(img_bucket_options);

		mongocxx::gridfs::uploader img_uploader = img_bucket.open_upload_stream(
			TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetImageFilename(
				Timestamp, Img.Metadata.Label, Img.Metadata.ViewType)));

		img_uploader.write(reinterpret_cast<const uint8_t*>(Img.Data.GetData()), static_cast<uint32_t>(Img.Data.Num()));

		mongocxx::result::gridfs::upload upload_result = img_uploader.close();
		
		bson_img_doc.append(kvp("img_id", upload_result.id()));
		

		// Add to array
		bson_arr.append(bson_img_doc);
	}

	// Avoid inserting empty entries
	if (!bson_arr.view().empty())
	{
		// Write the timestamp
		bson_doc.append(kvp("timestamp", bsoncxx::types::b_double{ Timestamp }));
		bson_doc.append(kvp("images", bson_arr));
		try
		{
			mongo_coll.insert_one(bson_doc.view());
		}
		catch (const std::exception& xcp)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
				TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		}
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
		//mongocxx::options::ssl ssl_options;

		// If the server certificate is not signed by a well-known CA,
		// you can set a custom CA file with the `ca_file` option.
		// ssl_options.ca_file("/path/to/custom/cert.pem");

		// If you want to disable certificate verification, you
		// can set the `allow_invalid_certificates` option.
		// ssl_options.allow_invalid_certificates(true);

		//client_options.ssl_opts(ssl_options);

		// Create the connection URI
		FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);

		// Create the mongo client/connection
		// drivers are designed to succeed during client construction,
		// regardless of the state of servers
		// i.e. can return true even if it is no connected to the server
		//mongo_conn = mongocxx::client{ mongocxx::uri {} };
		//mongo_conn = mongocxx::client{ mongocxx::uri {}, client_options };
		mongo_conn = mongocxx::client { mongocxx::uri {TCHAR_TO_UTF8(*Uri)}, client_options};
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

		if (!mongo_db.has_collection(TCHAR_TO_UTF8(*EpisodeId)))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Collestion %s does not exist in %s"),
				TEXT(__FUNCTION__), __LINE__, *EpisodeId, *DBName);
		}
		//mongo_coll = mongo_db[TCHAR_TO_UTF8(*EpisodeId)];
		mongo_coll = mongo_db["test"];

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

// Re-create indexes
bool USLVisImageWriterMongo::CreateIndexes()
{
#if SL_WITH_LIBMONGO
	if (!bIsInit)
	{
		return false;
	}

	// Create indexes on the database
	try
	{
		/*mongocxx::options::index index_options{};*/
		mongo_coll.create_index(bsoncxx::builder::basic::make_document(kvp("timestamp", 1))/*, index_options*/);
		return true;
	}
	catch (const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		return false;
	}
#else
	return false;
#endif //SL_WITH_LIBMONGO
}
