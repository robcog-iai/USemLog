// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongoCxx.h"
#if SLVIS_WITH_LIBMONGO
THIRD_PARTY_INCLUDES_START
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/gridfs/bucket.hpp>
#include <string.h>
#include <iostream>
THIRD_PARTY_INCLUDES_END
using namespace mongocxx;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
#endif //SLVIS_WITH_LIBMONGO

#include <mongocxx/instance.hpp>

#define SLVIS_MIN_TIME_OFFSET 4.5f

// Ctor
USLVisImageWriterMongoCxx::USLVisImageWriterMongoCxx()
{
	mongocxx::instance::current();
	bIsInit = false;
}

// Dtor
USLVisImageWriterMongoCxx::~USLVisImageWriterMongoCxx()
{
}

// Init
void USLVisImageWriterMongoCxx::Init(const FSLVisImageWriterParams& InParams)
{
	bIsInit = USLVisImageWriterMongoCxx::Connect(InParams.Location, InParams.EpisodeId, InParams.ServerIp, InParams.ServerPort);
}

// Finish
void USLVisImageWriterMongoCxx::Finish()
{
	if (bIsInit)
	{
		// Re-create the indexes
		USLVisImageWriterMongoCxx::CreateIndexes();
		bIsInit = false;
	}
}

// Write the images at the timestamp
void USLVisImageWriterMongoCxx::Write(float Timestamp, const TArray<FSLVisImageData>& ImagesData)
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
		bson_img_doc.append(kvp("type", TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetViewTypeName(Img.Metadata.ViewType))));

		// Add image resolution
		bsoncxx::builder::basic::document bson_res_doc{};
		bson_res_doc.append(kvp("x", bsoncxx::types::b_int32{Img.Metadata.ResX}));
		bson_res_doc.append(kvp("y", bsoncxx::types::b_int32{Img.Metadata.ResY}));
		bson_img_doc.append(kvp("res", bson_res_doc));

		// Add binary data
		//bson_img_doc.append(kvp("data", types::b_binary{
		//	binary_sub_type::k_binary,
		//	static_cast<uint32_t>(Img.Data.Num()),
		//	reinterpret_cast<const uint8_t*>(Img.Data.GetData())}));

		try
		{
			bsoncxx::builder::basic::document meta_doc{};
			meta_doc.append(kvp("x_meta", bsoncxx::types::b_int32{ 32 }));
			options::gridfs::upload upload_options;
			upload_options.metadata(meta_doc.view());
			FString ImageName = ISLVisImageWriterInterface::GetImageFilename(Timestamp, Img.Metadata.Label, Img.Metadata.ViewType);
			
			// Create gridfs bucket uploader with options
			gridfs::uploader gridfs_uploader = gridfs_bucket.open_upload_stream(TCHAR_TO_UTF8(*ImageName), upload_options);

			// Write data to uploader
			gridfs_uploader.write(reinterpret_cast<const uint8_t*>(Img.Data.GetData()), static_cast<uint32_t>(Img.Data.Num()));

			// Close uploader and write the id of the written data object
			result::gridfs::upload upload_result = gridfs_uploader.close();
			bson_img_doc.append(kvp("img_id", upload_result.id()));
		}
		catch (const std::exception& xcp)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
				TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		}

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
			if (bCreateNewDocument)
			{
				mongo_coll.insert_one(bson_doc.view());
			}
			else
			{
				// TODO
				// Insert at cached document _id
				mongo_coll.insert_one(bson_doc.view());
			}
		}
		catch (const std::exception& xcp)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
				TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		}
	}
#endif //SLVIS_WITH_LIBMONGO
}

// Skip the current timestamp (images already inserted)
bool USLVisImageWriterMongoCxx::ShouldSkipThisTimestamp(float Timestamp)
{
	// Check if a new entry should be created only for the images abs(world_state.ts - ts) > threshold
	// if true, cache the result and use it in write
	// return false, so the images at are rendered at this timestamp
	// if false
	  // check if there is already an images entry in the world state
		 // if true, return true (we can skip rendering the images)
		 // if false, return false (we need to render the images) 
			//  cache object id, this is where the images will be inserted	
#if SLVIS_WITH_LIBMONGO
	// Check nearest world state entry  WS_before <--ts_diff_before-- TS --ts_diff_after--> WS_after
	float SmallestDiff = -1.f;

	// Get world state document before the given timestamp
	bsoncxx::document::value find_query_before = make_document(
		kvp("timestamp", make_document(kvp("$lte", Timestamp))));
	// Sort results descendingly (we are interested in the first before the input ts)
	mongocxx::options::find find_opt_desc{};
	find_opt_desc.sort(make_document(kvp("timestamp", -1)));

	// Get document
	auto before_doc = mongo_coll.find_one(find_query_before.view(), find_opt_desc);
	if (before_doc)
	{
		// Continue if there were no images already added here (it might happen due to the threshold
		bsoncxx::document::element imgs_el = before_doc->view()["images"];
		if (!imgs_el)
		{
			bsoncxx::document::element ts_el = before_doc->view()["timestamp"];
			if (ts_el && ts_el.type() == bsoncxx::type::k_double)
			{
				float CurrDiff = FMath::Abs(Timestamp - ts_el.get_double());
				if (CurrDiff < SLVIS_MIN_TIME_OFFSET)
				{
					bsoncxx::document::element id_el = before_doc->view()["_id"];
					if (id_el && id_el.type() == bsoncxx::type::k_oid)
					{
						insertion_doc_id = id_el.get_oid();
						SmallestDiff = CurrDiff;
						UE_LOG(LogTemp, Warning, TEXT("%s::%d TsDiff=%f"), TEXT(__FUNCTION__), __LINE__, SmallestDiff);
					}
				}
			}
		}
	}

	// Get after query
	bsoncxx::document::value find_query_after = make_document(
		kvp("timestamp", make_document(kvp("$gt", Timestamp))));
	// Sort results ascendingly (we are interested in the first after the input ts) 
	mongocxx::options::find find_opt_asc{};
	find_opt_asc.sort(make_document(kvp("timestamp", 1)));

	// Get document
	auto after_doc = mongo_coll.find_one(find_query_after.view(), find_opt_asc);
	if (after_doc)
	{		
		// Continue if there were no images already added here (it might happen due to the threshold
		bsoncxx::document::element imgs_el = after_doc->view()["images"];
		if (!imgs_el)
		{
			bsoncxx::document::element ts_el = after_doc->view()["timestamp"];
			if (ts_el && ts_el.type() == bsoncxx::type::k_double)
			{
				float CurrDiff = FMath::Abs(Timestamp - ts_el.get_double());
				if (CurrDiff < SLVIS_MIN_TIME_OFFSET)
				{
					bsoncxx::document::element id_el = after_doc->view()["_id"];
					if (id_el && id_el.type() == bsoncxx::type::k_oid)
					{
						// Check against previous result
						if (SmallestDiff < 0 || CurrDiff < SmallestDiff)
						{
							// Other result is invalid or it is valid but current diff is smaller
							insertion_doc_id = id_el.get_oid();
							SmallestDiff = CurrDiff;
							UE_LOG(LogTemp, Warning, TEXT("%s::%d TsDiff=%f"), TEXT(__FUNCTION__), __LINE__, SmallestDiff);
							bCreateNewDocument = false;
							return false;
						}
						else
						{
							// Use other result (which is valid since we check if the diff is not negative)
							UE_LOG(LogTemp, Warning, TEXT("%s::%d TsDiff=%f"), TEXT(__FUNCTION__), __LINE__, SmallestDiff);
							bCreateNewDocument = false;
							return false;
						}
					}
				}
			}
		}
	}
	// todo with returns

#endif //SLVIS_WITH_LIBMONGO
	return false;
}

// Connect to the database (returns true if there is a server running and we are connected)
bool USLVisImageWriterMongoCxx::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Params: DBName=%s; Collection=%s; IP=%s; Port=%d;"),
		TEXT(__FUNCTION__), __LINE__, *DBName, *EpisodeId, *ServerIp, ServerPort);

#if SLVIS_WITH_LIBMONGO
	try
	{
		// Get current mongo instance, or create a new one (static variable)
		instance::current();

		// Client connection options
		options::client client_options;
		//options::ssl ssl_options;

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
		mongo_conn = client{uri{TCHAR_TO_UTF8(*Uri)}, client_options};
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
			// TODO abort in this case
		}
		mongo_coll = mongo_db[TCHAR_TO_UTF8(*EpisodeId)];

		// Create the gridfs bucket for uploading the images data
		gridfs_bucket = mongo_db.gridfs_bucket(
			options::gridfs::bucket{}/*.bucket_name(TCHAR_TO_UTF8(*EpisodeId))*/);
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

// Not needed if the index already exists (it gets updated for every new entry)
bool USLVisImageWriterMongoCxx::CreateIndexes()
{
	if (!bIsInit)
	{
		return false;
	}

#if SLVIS_WITH_LIBMONGO
	// Create indexes on timestamp
	try
	{
		// todo check if index already exists
		/*options::index index_options{};*/
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
#endif //SLVIS_WITH_LIBMONGO
}
