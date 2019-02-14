// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongoC.h"

// Ctor
USLVisImageWriterMongoC::USLVisImageWriterMongoC()
{
	bIsInit = false;
	ws_oid_str[0] = 0;
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
	ws_oid_str[0] = 0;
	//bson_free(ws_oid2);
	//ws_oid2 = nullptr;
	bCreateNewEntry = false;
	TimeRange = InParams.SkipNewEntryTolerance;
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d Images data is empty, nothing to write.."),
			TEXT(__FUNCTION__), __LINE__);
		return;
	}

#if SLVIS_WITH_LIBMONGO
	bson_error_t error;	

	if (bCreateNewEntry)
	{
		// Create a new database entry for the data
		UE_LOG(LogTemp, Warning, TEXT("%s::%d !!! WRITE !!! New entry"), TEXT(__FUNCTION__), __LINE__);

		// Document to store the images data
		bson_t* imgs_doc = bson_new();

		// Add timestamp
		BSON_APPEND_DOUBLE(imgs_doc, "timestamp", Timestamp);

		// Save images data to gridfs, and create a bson entry
		USLVisImageWriterMongoC::SaveImagesReturnEntry(ImagesData, imgs_doc);

		// Insert imgs data
		if (!mongoc_collection_insert_one(collection, imgs_doc, NULL, NULL, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
				TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		}
		// Clean up allocated bson documents.
		bson_destroy(imgs_doc);
	}
	else
	{
		// Create a new database entry for the data
		UE_LOG(LogTemp, Warning, TEXT("%s::%d !!! WRITE !!! Update entry"), TEXT(__FUNCTION__), __LINE__);

		// Update existing entry with the imgs data
		if (ws_oid_str[0] != 0 && bson_oid_is_valid(ws_oid_str, strlen(ws_oid_str)))
		{
			bson_oid_t ws_oid;
			bson_t* double_check_query;

			// Get the cached oid
			bson_oid_init_from_string(&ws_oid, ws_oid_str);

			double_check_query = BCON_NEW("$and", "[",
				"{", "_id", BCON_OID(&ws_oid), "}",
				"{", "images", "{", "$exists", BCON_BOOL(false), "}", "}",
				"]");

			// Double check that the id is valid and there are no existing previous images in the entry,
			// this avoids adding unnecessary img data to the database if the update (second) query fails
			if (mongoc_collection_find_with_opts(collection, double_check_query, NULL, NULL))
			{
				bson_t* imgs_doc = bson_new();
				bson_t* update_doc = NULL;
				bson_t *update_query = NULL;

				// Add timestamp
				BSON_APPEND_DOUBLE(imgs_doc, "timestamp2", Timestamp);
				//BSON_APPEND_OID(imgs_doc, "oid2", ws_oid2); // TODO test storing the oid and not a string

				// Save images data to gridfs, and create a bson entry
				USLVisImageWriterMongoC::SaveImagesReturnEntry(ImagesData, imgs_doc);

				// Add the images data to the update document
				update_doc = BCON_NEW("$set", BCON_DOCUMENT(imgs_doc));

				// Add the images data to the given oid
				update_query = BCON_NEW("_id", BCON_OID(&ws_oid));
				if (!mongoc_collection_update_one(collection, update_query, update_doc, NULL, NULL, &error))
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
						TEXT(__FUNCTION__), __LINE__, *FString(error.message));
				}
				bson_destroy(imgs_doc);
				bson_destroy(update_doc);
				bson_destroy(update_query);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Entry _id=%s has previous images, skipping write. (This should not happen)"), 
					TEXT(__FUNCTION__), __LINE__, *FString(ws_oid_str));
			}
			bson_destroy(double_check_query);			
		}
		else 
		{
			// _id invalid or empty (this should not happen because ShouldSkipThisFrame checks for these cases)
			UE_LOG(LogTemp, Error, TEXT("%s::%d !!! _id is empty or invalid, this should never happen.."),
				TEXT(__FUNCTION__), __LINE__);
		}
	}
#endif //SLVIS_WITH_LIBMONGO
}

// Skip the current timestamp (images already inserted)
bool USLVisImageWriterMongoC::ShouldSkipThisFrame(float Timestamp)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Writer is not init"), TEXT(__FUNCTION__), __LINE__);
		return true;
	}
#if SLVIS_WITH_LIBMONGO
	// Invalidate previous document id
	//ws_oid_str = NULL;
	ws_oid_str[0] = 0;
	//bson_free(ws_oid2);
	//ws_oid2 = nullptr;
	
	FSLVisWorldStateEntryParams BeforeWS;
	USLVisImageWriterMongoC::GetWorldStateParamsAt(Timestamp, true, BeforeWS);

	FSLVisWorldStateEntryParams AfterWS;
	USLVisImageWriterMongoC::GetWorldStateParamsAt(Timestamp, false, AfterWS);

	// Flag if world state entries are valip for image data update
	bool bBeforeIsValidForUpdate = BeforeWS.bAllDataIsValid && (!BeforeWS.bContainsImageData) && (BeforeWS.TimeDistance < TimeRange);
	bool bAfterIsValidForUpdate = AfterWS.bAllDataIsValid && (!AfterWS.bContainsImageData) && (AfterWS.TimeDistance < TimeRange);
	
	// Check if a valid world state was found before the query timestamp
	if(bBeforeIsValidForUpdate)
	{
		if (bAfterIsValidForUpdate)
		{
			// Both valid (take the closest)
			strcpy_s(ws_oid_str, BeforeWS.TimeDistance < AfterWS.TimeDistance ? BeforeWS.oid_str : AfterWS.oid_str);
			bCreateNewEntry = false;
			return false;
		}
		else
		{
			// Before valid
			strcpy_s(ws_oid_str, BeforeWS.oid_str);
			bCreateNewEntry = false;
			return false;
		}
	}
	else
	{
		if (bAfterIsValidForUpdate)
		{
			// After valid
			strcpy_s(ws_oid_str, AfterWS.oid_str);
			bCreateNewEntry = false;
			return false;
		}
		else
		{
			// None valid for update, check if new entry should be created or to skip the frame
			if ((BeforeWS.bAllDataIsValid && BeforeWS.bContainsImageData && (BeforeWS.TimeDistance < TimeRange)))
			{
				// Skip frame if there is already image data in the time range
				bCreateNewEntry = false;
				return true;
			}
			else if ((AfterWS.bAllDataIsValid && AfterWS.bContainsImageData && (AfterWS.TimeDistance < TimeRange)))
			{
				// Skip frame if there is already image data in the time range
				bCreateNewEntry = false;
				return true;
			}
			else
			{
				// Create a new entry
				bCreateNewEntry = true;
				return false;
			}
		}
	}	
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

	// Clean up
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

#if SLVIS_WITH_LIBMONGO
// Save images to gridfs and return the bson entry
void USLVisImageWriterMongoC::SaveImagesReturnEntry(const TArray<FSLVisImageData>& ImagesData, bson_t* out_imgs_doc)
{
	// Init bson if needed
	if (!out_imgs_doc)
	{
		out_imgs_doc = bson_new();
	}

	bson_t arr_child;
	bson_t arr_obj_child;
	bson_t res_child;
	// Add image sub-documents to array
	BSON_APPEND_ARRAY_BEGIN(out_imgs_doc, "images", &arr_child);
	for (const auto& Img : ImagesData)
	{
		bson_oid_t img_file_id;
		// Write img data to gridfs
		if (USLVisImageWriterMongoC::SaveImageToGridFS(Img, &img_file_id))
		{
			// Create image sub-doc
			BSON_APPEND_DOCUMENT_BEGIN(&arr_child, "images", &arr_obj_child);
			BSON_APPEND_OID(&arr_obj_child, "img_file_id", (const bson_oid_t*)&img_file_id);
			BSON_APPEND_UTF8(&arr_obj_child, "label", TCHAR_TO_UTF8(*Img.Metadata.Label));
			BSON_APPEND_UTF8(&arr_obj_child, "type", TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetViewTypeName(Img.Metadata.ViewType)));
			BSON_APPEND_DOUBLE(&arr_obj_child, "replay_timestamp", Img.Metadata.ReplayTimestamp);

			// Add img resolution sub-sub-doc
			BSON_APPEND_DOCUMENT_BEGIN(&arr_obj_child, "res", &res_child);
			BSON_APPEND_DOUBLE(&res_child, "x", Img.Metadata.ResX);
			BSON_APPEND_DOUBLE(&res_child, "y", Img.Metadata.ResY);
			bson_append_document_end(&arr_obj_child, &res_child);

			bson_append_document_end(&arr_child, &arr_obj_child);
		}
	}
	bson_append_array_end(out_imgs_doc, &arr_child);
}

// Write image data to gridfs (return the oid of the file/entry)
bool USLVisImageWriterMongoC::SaveImageToGridFS(const FSLVisImageData& ImgData, bson_oid_t* out_oid)
{
	mongoc_gridfs_file_t *file;
	mongoc_gridfs_file_opt_t file_opt = { 0 };
	bson_t* metadata_doc;
	const bson_value_t* file_id_val;
	mongoc_iovec_t iov;
	bson_error_t error;

	metadata_doc = BCON_NEW(
		"label", BCON_UTF8(TCHAR_TO_UTF8(*ImgData.Metadata.Label)),
		"type", BCON_UTF8(TCHAR_TO_UTF8(*ISLVisImageWriterInterface::GetViewTypeName(ImgData.Metadata.ViewType))),
		"replay_timestamp", BCON_DOUBLE(ImgData.Metadata.ReplayTimestamp),
		"res", "{",
			"x", BCON_INT32(ImgData.Metadata.ResX),
			"y", BCON_INT32(ImgData.Metadata.ResY), "}"
	);

	file_opt.filename = TCHAR_TO_UTF8(*ISLVisImageWriterInterface::CreateImageFilename(
		ImgData.Metadata.ReplayTimestamp, ImgData.Metadata.Label, ImgData.Metadata.ViewType));
	file_opt.metadata = metadata_doc;

	// Create new file
	file = mongoc_gridfs_create_file(gridfs, &file_opt);

	// Set data binary and length
	iov.iov_base = (char*)(ImgData.BinaryData.GetData());
	iov.iov_len = ImgData.BinaryData.Num();

	// Write data to gridfs
	if (iov.iov_len != mongoc_gridfs_file_writev(file, &iov, 1, 0))
	{
		if (mongoc_gridfs_file_error(file, &error))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
				TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		}
		return false;
	}

	// Saves modifications to file to the MongoDB server
	if (!mongoc_gridfs_file_save(file))
	{
		mongoc_gridfs_file_error(file, &error);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		return false;
	}

	// Set the out oid
	file_id_val = mongoc_gridfs_file_get_id(file);
	bson_oid_copy(&file_id_val->value.v_oid, out_oid);

	// Clean up
	bson_destroy(metadata_doc);
	mongoc_gridfs_file_destroy(file);

	return true;
}
#endif //SLVIS_WITH_LIBMONGO