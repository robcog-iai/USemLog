// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongoC.h"

#define SLVIS_INSERT_TIME_TRESHOLD 4.5f

// Ctor
USLVisImageWriterMongoC::USLVisImageWriterMongoC()
{
	bIsInit = false;
}

// Dtor
USLVisImageWriterMongoC::~USLVisImageWriterMongoC()
{
#if SLVIS_WITH_LIBMONGO
	//Release our handles and clean up libmongoc
	mongoc_collection_destroy(collection);
	mongoc_database_destroy(database);
	mongoc_uri_destroy(uri);
	mongoc_client_destroy(client);
	mongoc_cleanup();
#endif //SLVIS_WITH_LIBMONGO
}

// Init
void USLVisImageWriterMongoC::Init(const FSLVisImageWriterParams& InParams)
{
	bIsInit = USLVisImageWriterMongoC::Connect(InParams.Location, InParams.EpisodeId, InParams.ServerIp, InParams.ServerPort);
}

// Finish
void USLVisImageWriterMongoC::Finish()
{
	if (bIsInit)
	{
		// Re-create the indexes
		USLVisImageWriterMongoC::CreateIndexes();
		bIsInit = false;
	}
}

// Write the images at the timestamp
void USLVisImageWriterMongoC::Write(float Timestamp, const TArray<FSLVisImageData>& ImagesData)
{	
	if (!(ImagesData.Num() > 0))
	{
		// Avoid writing empty images
		return;
	}

#if SLVIS_WITH_LIBMONGO
	bson_error_t error;
	bson_t* bson_doc = bson_new();
	bson_t arr_child;
	bson_t arr_obj_child;
	bson_t res_child;

	// Add timestamp
	BSON_APPEND_DOUBLE(bson_doc, "timestamp", Timestamp);

	// Add image sub-documents to array
	BSON_APPEND_ARRAY_BEGIN(bson_doc, "images", &arr_child);
	for (const auto& Img : ImagesData)
	{
		// Create image sub-doc
		BSON_APPEND_DOCUMENT_BEGIN(&arr_child, "images", &arr_obj_child);
		BSON_APPEND_UTF8(&arr_obj_child, "label", TCHAR_TO_UTF8(*Img.Metadata.Label));
		BSON_APPEND_UTF8(&arr_obj_child, "type", TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetViewTypeName(Img.Metadata.ViewType)));

			// Add img resolution sub-sub-doc
			BSON_APPEND_DOCUMENT_BEGIN(&arr_obj_child, "res", &res_child);
			BSON_APPEND_DOUBLE(&res_child, "x", Img.Metadata.ResX);
			BSON_APPEND_DOUBLE(&res_child, "y", Img.Metadata.ResY);
			bson_append_document_end(&arr_obj_child, &res_child);

		bson_append_document_end(&arr_child, &arr_obj_child);
	}
	bson_append_array_end(bson_doc, &arr_child);

	// Insert img data
	if (!mongoc_collection_insert_one(collection, bson_doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d mongo insert err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
	}


	// Clean up allocated bson documents.
	bson_destroy(bson_doc);

#endif //SLVIS_WITH_LIBMONGO
}

// Skip the current timestamp (images already inserted)
bool USLVisImageWriterMongoC::ShouldSkipThisTimestamp(float Timestamp)
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

#endif //SLVIS_WITH_LIBMONGO
	return false;
}

// Connect to the database (returns true if there is a server running and we are connected)
bool USLVisImageWriterMongoC::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Params: DBName=%s; Collection=%s; IP=%s; Port=%d;"),
		TEXT(__FUNCTION__), __LINE__, *DBName, *EpisodeId, *ServerIp, ServerPort);
#if SLVIS_WITH_LIBMONGO
	// Required to initialize libmongoc's internals	
	mongoc_init();

	bson_error_t error;
	
	// Safely create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);
	uri = mongoc_uri_new_with_error(TCHAR_TO_UTF8(*Uri), &error);
	if (!uri) 
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d failed to parse URI:%s, err msg:%s"),
			TEXT(__FUNCTION__), __LINE__, *Uri, *FString(error.message));
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client) 
	{
		return false;
	}

	// Register the application name so we can track it in the profile logs
	// on the server. This can also be done from the URI (see other examples).	 
	mongoc_client_set_appname(client, "slvis");

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));
	collection = mongoc_client_get_collection(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*EpisodeId));

	// Check server. Ping the "admin" database
	bson_t* command = BCON_NEW("ping", BCON_INT32(1));
	bson_t reply;
	bool retval = mongoc_client_command_simple(
		client, "admin", command, NULL, &reply, &error);
	if (!retval) 
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		return false;
	}
	char* str = bson_as_json(&reply, NULL);
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Ret=%s"),
		TEXT(__FUNCTION__), __LINE__, *FString(str));

	bson_destroy(&reply);
	bson_destroy(command);
	bson_free(str);

	return true;
#else
	return false;
#endif //SLVIS_WITH_LIBMONGO
}

// Re-create indexes
bool USLVisImageWriterMongoC::CreateIndexes()
{
#if SLVIS_WITH_LIBMONGO
	if (!bIsInit)
	{
		return false;
	}

	return false;
#else
	return false;
#endif //SLVIS_WITH_LIBMONGO
}
