// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongoC.h"

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
	mongoc_gridfs_destroy(gridfs);
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
	TimeRange = InParams.NewEntryTimeRange;
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
	// Avoid inserting empty array entries
	if (!(ImagesData.Num() > 0))
	{
		return;
	}

#if SLVIS_WITH_LIBMONGO

	bson_error_t error;

	// Iterate images
	for (const auto& Img : ImagesData)
	{
		//mongoc_stream_t *img_stream;
		mongoc_gridfs_file_t *img_file;
		mongoc_gridfs_file_opt_t img_file_opt = { 0 };
		bson_t* img_metadata_doc;
		//bson_value_t img_file_id;
		const bson_value_t* img_file_id_val;
		bson_oid_t img_file_oid;
		mongoc_iovec_t iov;

		//img_stream = mongoc_stream_file_new_for_path("C:/img.png", O_RDONLY, 0);
		//stream = mongoc_stream_file_new(0);

		img_metadata_doc = BCON_NEW(
			"label", BCON_UTF8(TCHAR_TO_UTF8(*Img.Metadata.Label)),
			"type", BCON_UTF8(TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetViewTypeName(Img.Metadata.ViewType))),
			"res","{",
				"x", BCON_INT32(Img.Metadata.ResX),
				"y", BCON_INT32(Img.Metadata.ResY),"}"
		);

		img_file_opt.filename = TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetImageFilename(Timestamp, Img.Metadata.Label, Img.Metadata.ViewType));
		img_file_opt.metadata = img_metadata_doc;

		// The driver generates the file_id
		//img_file = mongoc_gridfs_create_file_from_stream(gridfs, img_stream, &img_file_opt);
		img_file = mongoc_gridfs_create_file(gridfs, &img_file_opt);
		iov.iov_base = (char*)(Img.Data.GetData());
		iov.iov_len = Img.Data.Num();

		if (iov.iov_len != mongoc_gridfs_file_writev(img_file, &iov, 1, 0)) 
		{
			if (mongoc_gridfs_file_error(img_file, &error)) 
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
					TEXT(__FUNCTION__), __LINE__, *FString(error.message));
			}
		}

		//mongoc_gridfs_file_writev(img_file,)

		//img_file_id.value_type = BSON_TYPE_INT32;
		//img_file_id.value.v_int32 = 1;
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d id=%lld"), TEXT(__FUNCTION__), __LINE__, img_file_id.value.v_int32);

		//// optional: the following method specifies a file_id of any BSON type
		//if (!mongoc_gridfs_file_set_id(img_file, &img_file_id, &error)) 
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
		//		TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		//	//continue;
		//}

		//UE_LOG(LogTemp, Warning, TEXT("%s::%d after id=%lld"), TEXT(__FUNCTION__), __LINE__, img_file_id.value.v_int32);


		//img_id = mongoc_gridfs_file_get_id(img_file);
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d AAA after id=%lld"), TEXT(__FUNCTION__), __LINE__, img_id->value.v_int32);

		// Save the data
		if (!mongoc_gridfs_file_save(img_file)) 
		{
			mongoc_gridfs_file_error(img_file, &error);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
				TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		}
		img_file_id_val = mongoc_gridfs_file_get_id(img_file);
		//if (!img_file_id_val->value_type == BSON_TYPE_OID)
		// err
		bson_oid_copy(&img_file_id_val->value.v_oid, &img_file_oid);
		
		char oidstr[25];
		bson_oid_to_string(&img_file_id_val->value.v_oid, oidstr);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d OID.:%s"),
			TEXT(__FUNCTION__), __LINE__, *FString(oidstr));

		// Clean up
		bson_destroy(img_metadata_doc);
		mongoc_gridfs_file_destroy(img_file);

		UE_LOG(LogTemp, Warning, TEXT("\t\t %s::%d *** "), TEXT(__FUNCTION__), __LINE__);
	}

	UE_LOG(LogTemp, Warning, TEXT("\t\t \t\t %s::%d  !!!! *** "), TEXT(__FUNCTION__), __LINE__);
	//mongoc_stream_destroy(stream);
	


	/*********************************************************************/
	/*
	// Insert to a current entry or create a new one
	if (bCreateNewEntry)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d NEW ENTRY SHALL BE CREATED"), TEXT(__FUNCTION__), __LINE__);
	}
	else
	{
		if (ws_oid_str && bson_oid_is_valid(ws_oid_str, strlen(ws_oid_str)))
		{
			bson_oid_t ws_oid;
			bson_oid_init_from_string(&ws_oid, ws_oid_str);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d oid=%s"),
				TEXT(__FUNCTION__), __LINE__, *FString(ws_oid_str));
		}
		else 
		{

			// Check if there is img data at the exact timestamp, if no add new entry
			UE_LOG(LogTemp, Warning, TEXT("%s::%d ws id invalid new entry shall be created"), TEXT(__FUNCTION__), __LINE__);
		}
	}
	*/
	/************************************************************************/


	//bson_error_t error;
	//bson_t* bson_doc = bson_new();
	//bson_t arr_child;
	//bson_t arr_obj_child;
	//bson_t res_child;

	//// Add timestamp
	//BSON_APPEND_DOUBLE(bson_doc, "timestamp", Timestamp);

	//// Add image sub-documents to array
	//BSON_APPEND_ARRAY_BEGIN(bson_doc, "images", &arr_child);
	//for (const auto& Img : ImagesData)
	//{
	//	// Create image sub-doc
	//	BSON_APPEND_DOCUMENT_BEGIN(&arr_child, "images", &arr_obj_child);
	//	BSON_APPEND_UTF8(&arr_obj_child, "label", TCHAR_TO_UTF8(*Img.Metadata.Label));
	//	BSON_APPEND_UTF8(&arr_obj_child, "type", TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetViewTypeName(Img.Metadata.ViewType)));

	//		// Add img resolution sub-sub-doc
	//		BSON_APPEND_DOCUMENT_BEGIN(&arr_obj_child, "res", &res_child);
	//		BSON_APPEND_DOUBLE(&res_child, "x", Img.Metadata.ResX);
	//		BSON_APPEND_DOUBLE(&res_child, "y", Img.Metadata.ResY);
	//		bson_append_document_end(&arr_obj_child, &res_child);

	//	bson_append_document_end(&arr_child, &arr_obj_child);
	//}
	//bson_append_array_end(bson_doc, &arr_child);

	//// Insert img data
	//if (!mongoc_collection_insert_one(collection, bson_doc, NULL, NULL, &error))
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d mongo insert err.: %s"),
	//		TEXT(__FUNCTION__), __LINE__, *FString(error.message));
	//}


	//// Clean up allocated bson documents.
	//bson_destroy(bson_doc);
#endif //SLVIS_WITH_LIBMONGO
}

// Skip the current timestamp (images already inserted)
bool USLVisImageWriterMongoC::ShouldSkipThisTimestamp(float Timestamp)
{
#if SLVIS_WITH_LIBMONGO
	// Invalidate previous document id
	ws_oid_str = NULL;
	
	UE_LOG(LogTemp, Error, TEXT("%s::%d Timestamp=%f"), TEXT(__FUNCTION__), __LINE__, Timestamp);

	FSLVisWorldStateEntryParams BeforeWS;
	USLVisImageWriterMongoC::GetWorldStateParamsAt(Timestamp, true, BeforeWS);
	UE_LOG(LogTemp, Warning, TEXT("%s::%d BeforeParams=%s"), TEXT(__FUNCTION__), __LINE__, *BeforeWS.ToString());

	FSLVisWorldStateEntryParams AfterWS;
	USLVisImageWriterMongoC::GetWorldStateParamsAt(Timestamp, false, AfterWS);
	UE_LOG(LogTemp, Error, TEXT("%s::%d AfterParams=%s"), TEXT(__FUNCTION__), __LINE__, *AfterWS.ToString());

	// Flag if world state entries are valip for image data update
	bool bBeforeIsValidForUpdate = BeforeWS.bAllDataIsValid && (!BeforeWS.bContainsImageData) && (BeforeWS.TimeDistance < TimeRange);
	bool bAfterIsValidForUpdate = AfterWS.bAllDataIsValid && (!AfterWS.bContainsImageData) && (AfterWS.TimeDistance < TimeRange);

	// Check if a valid world state was found before the query timestamp
	if(bBeforeIsValidForUpdate)
	{
		if (bAfterIsValidForUpdate)
		{
			// Both valid (take the closest)
			ws_oid_str = BeforeWS.TimeDistance < AfterWS.TimeDistance ? BeforeWS.oid_str : AfterWS.oid_str;
			bCreateNewEntry = false;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
			return false;
		}
		else
		{
			// Before valid
			ws_oid_str = BeforeWS.oid_str;
			bCreateNewEntry = false;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
			return false;
		}
	}
	else
	{
		if (bAfterIsValidForUpdate)
		{
			// After valid
			ws_oid_str = AfterWS.oid_str;
			bCreateNewEntry = false;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
			return false;
		}
		else
		{
			// None valid for update, check if new entry should be created or to skip the frame
			if ((BeforeWS.bAllDataIsValid && BeforeWS.bContainsImageData) 
				|| (BeforeWS.bAllDataIsValid && BeforeWS.bContainsImageData))
			{
				// Skip frame if there is already image data in the time range
				bCreateNewEntry = false;
				UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
				return true;
			}
			else
			{
				// Create a new entry
				bCreateNewEntry = true;
				UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
				return false;
			}
		}
	}


//	// Check if a new entry should be created only for the images abs(world_state.ts - ts) > threshold
//	// if true, cache the result and use it in write
//	// return false, so the images at are rendered at this timestamp
//	// if false
//	  // check if there is already an images entry in the world state
//		 // if true, return true (we can skip rendering the images)
//		 // if false, return false (we need to render the images) 
//			//  cache object id, this is where the images will be inserted	
//	bson_t* filter_before;
//	bson_t* opts_before;
//	bson_t* filter_after;
//	bson_t* opts_after;
//	mongoc_cursor_t* cursor;
//	bson_iter_t iter;
//	const bson_t* world_state_doc;
//	const bson_oid_t* world_state_id_cache= nullptr;
//
//	// -1.0 means no valid entry found, a new entry will be created at the given timestamp
//	float CachedDiff = -1.0;
//	
//	// Query for the closest document before the given timestamp
//	filter_before = BCON_NEW("timestamp", "{", "$lte", BCON_DOUBLE(Timestamp), "}");
//	opts_before = BCON_NEW(
//		"limit", BCON_INT64(1),
//		"sort", "{", "timestamp", BCON_INT32(-1), "}"/*,
//		"projection", "{", "timestamp", BCON_BOOL(true), "}"*/
//		);
//
//	// Check the result of the query
//	cursor = mongoc_collection_find_with_opts(collection, filter_before, opts_before, NULL);
//	if(mongoc_cursor_next(cursor, &world_state_doc)) 
//	{
//		// Get the entry timestamp
//		if (bson_iter_init_find(&iter, world_state_doc, "timestamp") && BSON_ITER_HOLDS_DOUBLE(&iter))
//		{
//			// Check if difference is in range
//			float CurrTs = (float)bson_iter_double(&iter);
//			float CurrDiff = FMath::Abs(Timestamp - CurrTs);
//			if (CurrDiff < Range)
//			{
//				// Make sure the world state does not have any previous images stored
//				if (!bson_iter_init_find(&iter, world_state_doc, "images"))
//				{
//					// Cache the current difference and the document _id
//					CachedDiff = CurrDiff;
//
//					// Cache the document id
//					if (bson_iter_init_find(&iter, world_state_doc, "_id") && BSON_ITER_HOLDS_OID(&iter))
//					{
//						//char oidstr[25];
//						//bson_oid_copy(bson_iter_oid(&iter), ws_id);
//						//bson_oid_to_string(ws_id, oidstr);
//						//UE_LOG(LogTemp, Warning, TEXT("%s::%d Before oid=%s diff=%f"),
//						//	TEXT(__FUNCTION__), __LINE__, *FString(oidstr), Diff);
//						char oidstr[25];
//						const bson_oid_t* oid = bson_iter_oid(&iter);
//						bson_oid_to_string(oid, oidstr);
//						UE_LOG(LogTemp, Warning, TEXT("%s::%d Before oid=%s ts=%f diff=%f"),
//							TEXT(__FUNCTION__), __LINE__, *FString(oidstr), CurrTs, CurrDiff);
//					}
//				}
//			}
//		}
//	}
//
//
//	// Query for the closest document after the given timestamp
//	filter_after = BCON_NEW("timestamp", "{", "$gt", BCON_DOUBLE(Timestamp), "}");
//	opts_after = BCON_NEW(
//		"limit", BCON_INT64(1),
//		"sort", "{", "timestamp", BCON_INT32(1), "}"/*,
//		"projection", "{", "timestamp", BCON_BOOL(true), "}"*/
//	);
//
//	// Check the result of the query
//	cursor = mongoc_collection_find_with_opts(collection, filter_after, opts_after, NULL);
//	if (mongoc_cursor_next(cursor, &world_state_doc))
//	{
//		// Get the entry timestamp
//		if (bson_iter_init_find(&iter, world_state_doc, "timestamp") && BSON_ITER_HOLDS_DOUBLE(&iter))
//		{
//			// Check if difference is in range
//			float CurrTs = (float)bson_iter_double(&iter);
//			float CurrDiff = FMath::Abs(Timestamp - CurrTs);
//			if (CurrDiff < Range)
//			{
//				// Make sure the world state does not have any previous images stored
//				if (!bson_iter_init_find(&iter, world_state_doc, "images"))
//				{
//					// Check if previous entry is valid
//					if (CachedDiff >= 0.f)
//					{
//						// Choose the closest entry to update
//						if (CurrDiff < CachedDiff)
//						{
//							// Use this as update entry, cache the document id
//							if (bson_iter_init_find(&iter, world_state_doc, "_id") && BSON_ITER_HOLDS_OID(&iter))
//							{
//								char oidstr[25];
//								const bson_oid_t* oid = bson_iter_oid(&iter);
//								bson_oid_to_string(oid, oidstr);
//								UE_LOG(LogTemp, Error, TEXT("%s::%d After oid=%s ts=%f diff=%f"),
//									TEXT(__FUNCTION__), __LINE__, *FString(oidstr), CurrTs, CurrDiff);
//							}
//						}
//					}
//					else
//					{
//						// Use this as update entry, cache the document id
//						if (bson_iter_init_find(&iter, world_state_doc, "_id") && BSON_ITER_HOLDS_OID(&iter))
//						{
//							char oidstr[25];
//							const bson_oid_t* oid = bson_iter_oid(&iter);
//							bson_oid_to_string(oid, oidstr);
//							UE_LOG(LogTemp, Error, TEXT("%s::%d After oid=%s ts=%f diff=%f"),
//								TEXT(__FUNCTION__), __LINE__, *FString(oidstr), CurrTs, CurrDiff);
//						}
//					}
//				}
//			}
//		}
//	}
//
//	UE_LOG(LogTemp, Error, TEXT("%s::%d* * * * * * ** * * * * * *"), TEXT(__FUNCTION__), __LINE__);
//
//
//	mongoc_cursor_destroy(cursor);
//	bson_destroy(filter_before);
//	bson_destroy(opts_before);
//	bson_destroy(filter_after);
//	bson_destroy(opts_after);
	
#endif //SLVIS_WITH_LIBMONGO
	return false;
}

// Connect to the database (returns true if there is a server running and we are connected)
bool USLVisImageWriterMongoC::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
#if SLVIS_WITH_LIBMONGO
	// Required to initialize libmongoc's internals	
	mongoc_init();

	// Stores any arror that might appear during the connection
	bson_error_t error;
	
	// Safely create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);
	uri = mongoc_uri_new_with_error(TCHAR_TO_UTF8(*Uri), &error);
	if (!uri) 
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			TEXT(__FUNCTION__), __LINE__, *Uri, *FString(error.message));
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client) 
	{
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SLVis_" + EpisodeId)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));
	collection = mongoc_client_get_collection(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*EpisodeId));

	// Check server. Ping the "admin" database
	bson_t* server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
	//bson_t reply;
	if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		return false;
	}

	// Create a gridfs handle in test prefixed by fs */
	gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*EpisodeId), &error);
	if (!gridfs)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			TEXT(__FUNCTION__), __LINE__, *Uri, *FString(error.message));
		return false;
	}

	bson_destroy(server_ping_cmd);
	return true;
#else
	return false;
#endif //SLVIS_WITH_LIBMONGO
}

// Get parameters about the closest entry to the given timestamp
void USLVisImageWriterMongoC::GetWorldStateParamsAt(float InTimestamp, bool bSearchBeforeTimestamp, FSLVisWorldStateEntryParams& OutParams)
{
#if SLVIS_WITH_LIBMONGO
	bson_t* filter;
	bson_t* opts;
	mongoc_cursor_t* cursor;
	bson_iter_t iter;
	const bson_t* world_state_doc;

	// Query for the closest document before or after the given timestamp
	if (bSearchBeforeTimestamp)
	{
		filter = BCON_NEW("timestamp", "{", "$lte", BCON_DOUBLE(InTimestamp), "}");
		opts = BCON_NEW(
			"limit", BCON_INT64(1),
			"sort", "{", "timestamp", BCON_INT32(-1), "}"/*,
			"projection", "{", "timestamp", BCON_BOOL(true), "}"*/
		);
	}
	else 
	{
		filter = BCON_NEW("timestamp", "{", "$gt", BCON_DOUBLE(InTimestamp), "}");
		opts = BCON_NEW(
			"limit", BCON_INT64(1),
			"sort", "{", "timestamp", BCON_INT32(1), "}"/*,
			"projection", "{", "timestamp", BCON_BOOL(true), "}"*/
		);
	}

	// Check the result of the query
	cursor = mongoc_collection_find_with_opts(collection, filter, opts, NULL);
	if (mongoc_cursor_next(cursor, &world_state_doc))
	{
		// Get the entry timestamp
		if (bson_iter_init_find(&iter, world_state_doc, "timestamp") && BSON_ITER_HOLDS_DOUBLE(&iter))
		{
			OutParams.Timestamp = (float)bson_iter_double(&iter);
			OutParams.TimeDistance = FMath::Abs(OutParams.Timestamp - InTimestamp);
			OutParams.bContainsImageData = bson_iter_init_find(&iter, world_state_doc, "images");
			if (bson_iter_init_find(&iter, world_state_doc, "_id") && BSON_ITER_HOLDS_OID(&iter))
			{
				bson_oid_to_string(bson_iter_oid(&iter), OutParams.oid_str);
				OutParams.bAllDataIsValid = true;
			}
		}
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(filter);
	bson_destroy(opts);
#endif //SLVIS_WITH_LIBMONGO
}

// Not needed if the index already exists (it gets updated for every new entry)
bool USLVisImageWriterMongoC::CreateIndexes()
{
	if (!bIsInit)
	{
		return false;
	}

#if SLVIS_WITH_LIBMONGO
	return false;
#else
	return false;
#endif //SLVIS_WITH_LIBMONGO
}
