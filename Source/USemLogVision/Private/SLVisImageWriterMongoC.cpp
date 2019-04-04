// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
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
#if SLVIS_WITH_LIBMONGO_C
	//Release our handles and clean up libmongoc
	mongoc_gridfs_destroy(gridfs);
	mongoc_collection_destroy(collection);
	mongoc_database_destroy(database);
	mongoc_uri_destroy(uri);
	mongoc_client_destroy(client);
	mongoc_cleanup();
#endif //SLVIS_WITH_LIBMONGO_C
}

// Init
void USLVisImageWriterMongoC::Init(const FSLVisImageWriterParams& InParams)
{
#if SLVIS_WITH_LIBMONGO_C
	ws_oid_str[0] = 0;
	//bson_free(ws_oid2);
	//ws_oid2 = nullptr;
#endif //SLVIS_WITH_LIBMONGO_C
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
void USLVisImageWriterMongoC::Write(const FSLVisStampedData& StampedData)
{	
	// Avoid inserting empty array entries
	if (!(StampedData.ViewsData.Num() > 0))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Images data is empty, nothing to write.."),
			TEXT(__FUNCTION__), __LINE__);
		return;
	}

#if SLVIS_WITH_LIBMONGO_C
	bson_error_t error;

	if (bCreateNewEntry)
	{
		// Create a new database entry for the data
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Writing a new entry.."),
			TEXT(__FUNCTION__), __LINE__);

		// Document to store the images data
		bson_t* doc = bson_new();

		// Add timestamp
		BSON_APPEND_DOUBLE(doc, "timestamp", StampedData.Timestamp);

		// Add the views 
		USLVisImageWriterMongoC::AddViewsDataToDoc(StampedData.ViewsData, doc);

		// Insert imgs data
		if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
				TEXT(__FUNCTION__), __LINE__, *FString(error.message));
			bson_destroy(doc);
		}
		// Clean up allocated bson documents.
		bson_destroy(doc);
	}
	else // Update existing entry
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Updating entry oid=%s"),
			TEXT(__FUNCTION__), __LINE__, *FString(ws_oid_str));

		// Update existing entry with the imgs data
		//if(ws_oid2)
		if (ws_oid_str[0] != 0 && bson_oid_is_valid(ws_oid_str, strlen(ws_oid_str)))
		{
			bson_oid_t ws_oid;
			bson_t* double_check_query;

			// Get the cached oid
			bson_oid_init_from_string(&ws_oid, ws_oid_str);

			double_check_query = BCON_NEW("$and", "[",
				"{", "_id", BCON_OID(&ws_oid), "}",
				"{", "views", "{", "$exists", BCON_BOOL(false), "}", "}",
				"]");

			// Double check that the id is valid and there are no existing previous images in the entry,
			// this avoids adding unnecessary img data to the database if the update (second) query fails
			if (mongoc_collection_find_with_opts(collection, double_check_query, NULL, NULL))
			{
				bson_t* doc = bson_new();
				bson_t* update_doc = NULL;
				bson_t *update_query = NULL;

				// Add timestamp
				BSON_APPEND_DOUBLE(doc, "render_timestamp", StampedData.Timestamp);

				// Save images data to gridfs, and create a bson entry
				USLVisImageWriterMongoC::AddViewsDataToDoc(StampedData.ViewsData, doc);

				// Add the images data to the update document
				update_doc = BCON_NEW("$set", BCON_DOCUMENT(doc));

				// Add the images data to the given oid
				update_query = BCON_NEW("_id", BCON_OID(&ws_oid));
				if (!mongoc_collection_update_one(collection, update_query, update_doc, NULL, NULL, &error))
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
						TEXT(__FUNCTION__), __LINE__, *FString(error.message));
				}
				// Clean up
				bson_destroy(doc);
				bson_destroy(update_doc);
				bson_destroy(update_query);
			}
			else
			{
				//char oid_str[25];
				//bson_oid_to_string(ws_oid2, oid_str);
				//UE_LOG(LogTemp, Error, TEXT("%s::%d Update OID=%s"), TEXT(__FUNCTION__), __LINE__, *FString(oid_str));
				UE_LOG(LogTemp, Error, TEXT("%s::%d Entry _id=%s already has views stored, skipping.. (This should not happen)"), 
					TEXT(__FUNCTION__), __LINE__, *FString(ws_oid_str));
			}
			bson_destroy(double_check_query);
		}
		else 
		{
			// _id invalid or empty (this should not happen because ShouldSkipThisFrame checks for these cases)
			UE_LOG(LogTemp, Error, TEXT("%s::%d Search entry _id is empty or invalid, this should not happen.."),
				TEXT(__FUNCTION__), __LINE__);
		}
	}
#endif //SLVIS_WITH_LIBMONGO_C
}

// Skip the current timestamp (images already inserted)
bool USLVisImageWriterMongoC::ShouldSkipThisFrame(float Timestamp)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Writer is not init, force skipping frame"), TEXT(__FUNCTION__), __LINE__);
		return true;
	}
#if SLVIS_WITH_LIBMONGO_C
	// Invalidate previous document id
	ws_oid_str[0] = 0;
	//bson_destroy(ws_oid2);

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
#endif //SLVIS_WITH_LIBMONGO_C
	return false;
}

// Connect to the database (returns true if there is a server running and we are connected)
bool USLVisImageWriterMongoC::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
#if SLVIS_WITH_LIBMONGO_C
	// Required to initialize libmongoc's internals	
	mongoc_init();

	// Stores any error that might appear during the connection
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
	bson_t* server_ping_cmd;
	server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
	if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		bson_destroy(server_ping_cmd);
		return false;
	}

	// Create a gridfs handle in test prefixed by fs */
	gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*EpisodeId), &error);
	if (!gridfs)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			TEXT(__FUNCTION__), __LINE__, *Uri, *FString(error.message));
		bson_destroy(server_ping_cmd);
		return false;
	}

	bson_destroy(server_ping_cmd);
	return true;
#else
	return false;
#endif //SLVIS_WITH_LIBMONGO_C
}

// Get parameters about the closest entry to the given timestamp
void USLVisImageWriterMongoC::GetWorldStateParamsAt(float InTimestamp, bool bSearchBeforeTimestamp, FSLVisWorldStateEntryParams& OutParams)
{
#if SLVIS_WITH_LIBMONGO_C
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
			OutParams.bContainsImageData = bson_iter_init_find(&iter, world_state_doc, "views");
			if (bson_iter_init_find(&iter, world_state_doc, "_id") && BSON_ITER_HOLDS_OID(&iter))
			{
				bson_oid_copy(bson_iter_oid(&iter), &OutParams.oid);
				bson_oid_to_string(bson_iter_oid(&iter), OutParams.oid_str);
				OutParams.bAllDataIsValid = true;
			}
		}
	}

	// Clean up
	mongoc_cursor_destroy(cursor);
	bson_destroy(filter);
	bson_destroy(opts);
#endif //SLVIS_WITH_LIBMONGO_C
}

// Not needed if the index already exists (it gets updated for every new entry)
bool USLVisImageWriterMongoC::CreateIndexes()
{
	if (!bIsInit)
	{
		return false;
	}
#if SL_WITH_LIBMONGO_C	

	bson_t index;
	char *index_name;

	bson_t index2;
	char *index_name2;

	bson_t index3;
	char *index_name3;

	bson_t* index_command;
	bson_error_t error;

	bson_init(&index);
	BSON_APPEND_INT32(&index, "views.id", 1);
	index_name = mongoc_collection_keys_to_index_string(&index);

	bson_init(&index2);
	BSON_APPEND_INT32(&index2, "views.entities.id", 1);
	index_name2 = mongoc_collection_keys_to_index_string(&index2);

	bson_init(&index3);
	BSON_APPEND_INT32(&index3, "views.entities.bones.class", 1);
	index_name3 = mongoc_collection_keys_to_index_string(&index3);

	index_command = BCON_NEW("createIndexes",
		BCON_UTF8(mongoc_collection_get_name(collection)),
		"indexes",
		"[",
			"{",
				"key",
				BCON_DOCUMENT(&index),
				"name",
				BCON_UTF8(index_name),
				//"unique",
				//BCON_BOOL(false),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&index2),
				"name",
				BCON_UTF8(index_name2),
				//"unique",
				//BCON_BOOL(false),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&index3),
				"name",
				BCON_UTF8(index_name3),
				//"unique",
				//BCON_BOOL(false),
			"}",
		"]");

	if (!mongoc_collection_write_command_with_opts(collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		bson_destroy(index_command);
		bson_free(index_name);
		return false;
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(index_name);
	bson_free(index_name2);
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

#if SLVIS_WITH_LIBMONGO_C
// Save images to gridfs and return the bson entry
void USLVisImageWriterMongoC::AddViewsDataToDoc(const TArray<FSLVisViewData>& ViewsData, bson_t* out_views_doc)
{
	// Init bson if needed
	if (!out_views_doc)
	{
		out_views_doc = bson_new();
	}

	bson_t view_arr;
	bson_t view_arr_obj;
	bson_t view_arr_obj_res;

	bson_t entity_arr;
	bson_t entity_arr_obj;
	bson_t obj_color;

	bson_t entity_bone_arr;
	bson_t entity_bone_arr_obj;

	bson_t img_arr;
	bson_t img_arr_obj;
	bson_oid_t img_file_id;

	char i_str[16];
	const char *i_key;
	uint32_t i = 0;

	char j_str[16];
	const char *j_key;
	uint32_t j = 0;

	char k_str[16];
	const char *k_key;
	uint32_t k = 0;

	char l_str[16];
	const char *l_key;
	uint32_t l = 0;

	// Create the views array
	BSON_APPEND_ARRAY_BEGIN(out_views_doc, "views", &view_arr);
	for (const auto& View : ViewsData)
	{
		// Start view child doc (name is irrelevant since it will show up as an array index)
		bson_uint32_to_string(i, &i_key, i_str, sizeof i_str);
		BSON_APPEND_DOCUMENT_BEGIN(&view_arr, i_key, &view_arr_obj);

			// Add child doc data
			BSON_APPEND_UTF8(&view_arr_obj, "class", TCHAR_TO_UTF8(*View.Class));
			BSON_APPEND_UTF8(&view_arr_obj, "id", TCHAR_TO_UTF8(*View.Id));

			// Add img resolution sub-sub-doc
			BSON_APPEND_DOCUMENT_BEGIN(&view_arr_obj, "res", &view_arr_obj_res);
			BSON_APPEND_DOUBLE(&view_arr_obj_res, "x", View.Resolution.X);
			BSON_APPEND_DOUBLE(&view_arr_obj_res, "y", View.Resolution.Y);
			bson_append_document_end(&view_arr_obj, &view_arr_obj_res);

			// Create the entities array
			j = 0;
			BSON_APPEND_ARRAY_BEGIN(&view_arr_obj, "entities", &entity_arr);
			// Add the static meshe entities
			for (const auto& Entity : View.SemanticEntities)
			{
				bson_uint32_to_string(j, &j_key, j_str, sizeof j_str);
				BSON_APPEND_DOCUMENT_BEGIN(&entity_arr, j_key, &entity_arr_obj);

					BSON_APPEND_UTF8(&entity_arr_obj, "id", TCHAR_TO_UTF8(*Entity.Id));
					BSON_APPEND_UTF8(&entity_arr_obj, "class", TCHAR_TO_UTF8(*Entity.Class));
					BSON_APPEND_UTF8(&entity_arr_obj, "mask_hex", TCHAR_TO_UTF8(*Entity.ColorHex));
					BSON_APPEND_INT32(&entity_arr_obj, "num_pixels", Entity.NumPixels);

					// Add color sub-sub-sub doc
					BSON_APPEND_DOCUMENT_BEGIN(&entity_arr_obj, "mask_color", &obj_color);
					BSON_APPEND_DOUBLE(&obj_color, "r", Entity.Color.R);
					BSON_APPEND_DOUBLE(&obj_color, "g", Entity.Color.G);
					BSON_APPEND_DOUBLE(&obj_color, "b", Entity.Color.B);
					BSON_APPEND_DOUBLE(&obj_color, "a", Entity.Color.A);
					bson_append_document_end(&entity_arr_obj, &obj_color);

				bson_append_document_end(&entity_arr, &entity_arr_obj);
				j++;
			}
			// Add the skeletal entities
			for (const auto& SkelEntity : View.SemanticSkelEntities)
			{
				bson_uint32_to_string(j, &j_key, j_str, sizeof j_str);
				BSON_APPEND_DOCUMENT_BEGIN(&entity_arr, j_key, &entity_arr_obj);

				BSON_APPEND_UTF8(&entity_arr_obj, "id", TCHAR_TO_UTF8(*SkelEntity.Id));
				BSON_APPEND_UTF8(&entity_arr_obj, "class", TCHAR_TO_UTF8(*SkelEntity.Class));

					l = 0;
					BSON_APPEND_ARRAY_BEGIN(&entity_arr_obj, "bones", &entity_bone_arr);
					// Add the skeletal bones
					for (const auto& BoneData : SkelEntity.BonesData)
					{
						bson_uint32_to_string(l, &l_key, l_str, sizeof l_str);
						BSON_APPEND_DOCUMENT_BEGIN(&entity_bone_arr, l_key, &entity_bone_arr_obj);

						BSON_APPEND_UTF8(&entity_bone_arr_obj, "class", TCHAR_TO_UTF8(*BoneData.Class));

						BSON_APPEND_UTF8(&entity_bone_arr_obj, "mask_hex", TCHAR_TO_UTF8(*BoneData.ColorHex));
						BSON_APPEND_INT32(&entity_bone_arr_obj, "num_pixels", BoneData.NumPixels);

						// Add color sub-sub-sub doc
						BSON_APPEND_DOCUMENT_BEGIN(&entity_bone_arr_obj, "mask_color", &obj_color);
						BSON_APPEND_DOUBLE(&obj_color, "r", BoneData.Color.R);
						BSON_APPEND_DOUBLE(&obj_color, "g", BoneData.Color.G);
						BSON_APPEND_DOUBLE(&obj_color, "b", BoneData.Color.B);
						BSON_APPEND_DOUBLE(&obj_color, "a", BoneData.Color.A);
						bson_append_document_end(&entity_bone_arr_obj, &obj_color);

						bson_append_document_end(&entity_bone_arr, &entity_bone_arr_obj);
						l++;
					}
					bson_append_array_end(&entity_arr_obj, &entity_bone_arr);


				bson_append_document_end(&entity_arr, &entity_arr_obj);
				j++;
			}

			bson_append_array_end(&view_arr_obj, &entity_arr);

			// Add gridfs data sub array
			k = 0;
			BSON_APPEND_ARRAY_BEGIN(&view_arr_obj, "images", &img_arr);
			for (const auto& ImgData : View.ImagesData)
			{
				if (USLVisImageWriterMongoC::SaveImageToGridFS(ImgData, &img_file_id))
				{
					bson_uint32_to_string(k, &k_key, k_str, sizeof k_str);
					BSON_APPEND_DOCUMENT_BEGIN(&img_arr, i_key, &img_arr_obj);

					BSON_APPEND_UTF8(&img_arr_obj, "type", TCHAR_TO_UTF8(*FSLVisHelper::GetRenderTypeAsString(ImgData.RenderType)));
					BSON_APPEND_OID(&img_arr_obj, "file_id", (const bson_oid_t*)&img_file_id);

					bson_append_document_end(&img_arr, &img_arr_obj);
					k++;
				}
			}
			bson_append_array_end(&view_arr_obj, &img_arr);

		// End view child doc
		bson_append_document_end(&view_arr, &view_arr_obj);
		i++;
	}
	bson_append_array_end(out_views_doc, &view_arr);
}

// Write image data to gridfs (return the oid of the file/entry)
bool USLVisImageWriterMongoC::SaveImageToGridFS(const FSLVisImageData& ImgData, bson_oid_t* out_oid)
{
	mongoc_gridfs_file_t *file;
	mongoc_gridfs_file_opt_t file_opt = { 0 };
	const bson_value_t* file_id_val; // TODO why is this not destroyed?
	mongoc_iovec_t iov;
	bson_error_t error;

	//bson_t* metadata_doc;
	//metadata_doc = BCON_NEW(
	//	"type", BCON_UTF8(TCHAR_TO_UTF8(*ImgData.RenderType))
	//);
	//file_opt.filename = "no_name";
	//file_opt.metadata = metadata_doc;

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
		mongoc_gridfs_file_destroy(file);
		return false;
	}

	// Saves modifications to file to the MongoDB server
	if (!mongoc_gridfs_file_save(file))
	{
		mongoc_gridfs_file_error(file, &error);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		mongoc_gridfs_file_destroy(file);
		return false;
	}

	// Set the out oid
	file_id_val = mongoc_gridfs_file_get_id(file);
	bson_oid_copy(&file_id_val->value.v_oid, out_oid);

	// Clean up
	//bson_destroy(metadata_doc);
	mongoc_gridfs_file_destroy(file);

	return true;
}
#endif //SLVIS_WITH_LIBMONGO_C