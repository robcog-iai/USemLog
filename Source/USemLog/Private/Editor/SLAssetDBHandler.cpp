// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLAssetDBHandler.h"

// Ctor
FSLAssetDBHandler::FSLAssetDBHandler() {}

// Connect to the database
bool FSLAssetDBHandler::Connect(const FString& DBName, const FString& CollName, const FString& ServerIp,
	uint16 ServerPort, bool bUploadAction, bool bOverwrite)
{
#if SL_WITH_LIBMONGO_C
	// Required to initialize libmongoc's internals	
	mongoc_init();

	// Stores any error that might appear during the connection
	bson_error_t error;

	// Safely create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);
	uri = mongoc_uri_new_with_error(TCHAR_TO_UTF8(*Uri), &error);
	if (!uri)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s; [Uri=%s]"),
			*FString(__func__), __LINE__, *FString(error.message), *Uri);
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create a mongo client.."), *FString(__func__), __LINE__);
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SLAsset_" + CollName)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Check if the collection already exists
	if (bUploadAction)
	{
		if (mongoc_database_has_collection(database, TCHAR_TO_UTF8(*CollName), &error))
		{
			if (bOverwrite)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Asset collection %s already exists, will be removed and overwritten.."),
					*FString(__func__), __LINE__, *CollName);
				if (!mongoc_collection_drop(mongoc_database_get_collection(database, TCHAR_TO_UTF8(*CollName)), &error))
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
						*FString(__func__), __LINE__, *FString(error.message));
					return false;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Asset collection %s already exists and should not be overwritten, skipping upload.."),
					*FString(__func__), __LINE__, *CollName);
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Creating collection %s.%s for uploading assets .."),
				*FString(__func__), __LINE__, *DBName, *CollName);
		}

		// Create a gridfs handle prefixed by fs */
		gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*CollName), &error);
		if (!gridfs)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
				*FString(__func__), __LINE__, *Uri, *FString(error.message));
			return false;
		}
	}
	else
	{
		// Download read/only
		if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*CollName), &error))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Asset collection %s does not exist, skipping download.."),
				*FString(__func__), __LINE__, *CollName);
			return false;
		}
	}
	collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*CollName));



	// Double check that the server is alive. Ping the "admin" database
	bson_t* server_ping_cmd;
	server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
	if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bson_destroy(server_ping_cmd);
		return false;
	}
	bson_destroy(server_ping_cmd);

	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Disconnect and clean db connection
void FSLAssetDBHandler::Disconnect() const
{
#if SL_WITH_LIBMONGO_C
	// Release handles and clean up mongoc
	if (uri)
	{
		mongoc_uri_destroy(uri);
	}
	if (client)
	{
		mongoc_client_destroy(client);
	}
	if (database)
	{
		mongoc_database_destroy(database);
	}
	if (collection)
	{
		mongoc_collection_destroy(collection);
	}
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}

// Create indexes on the inserted data
void FSLAssetDBHandler::CreateIndexes() const
{
	// TODO see if any indexes make sense, if not remove this
#if SL_WITH_LIBMONGO_C
	bson_t* index_command;
	bson_error_t error;

	bson_t index;
	bson_init(&index);
	BSON_APPEND_INT32(&index, "vision", 1);
	char* index_str = mongoc_collection_keys_to_index_string(&index);

	bson_t idx_id;
	bson_init(&idx_id);
	BSON_APPEND_INT32(&idx_id, "vision.views.id", 1);
	char* idx_id_str = mongoc_collection_keys_to_index_string(&idx_id);


	index_command = BCON_NEW("createIndexes",
		BCON_UTF8(mongoc_collection_get_name(collection)),
		"indexes",
		"[",
			"{",
				"key",
				BCON_DOCUMENT(&index),
				"name",
				BCON_UTF8(index_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_id),
				"name",
				BCON_UTF8(idx_id_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
		"]");

	if (!mongoc_collection_write_command_with_opts(collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(index_str);
	bson_free(idx_id_str);
	
#endif //SL_WITH_LIBMONGO_C
}

// Save image to gridfs, get the file oid and return true if succeeded
bool FSLAssetDBHandler::AddToGridFs(const TArray<uint8>& InData, bson_oid_t* out_oid) const
{
	mongoc_gridfs_file_t *file;
	mongoc_gridfs_file_opt_t file_opt = { 0 };
	const bson_value_t* file_id_val;
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
	iov.iov_base = (char*)(InData.GetData());
	iov.iov_len = InData.Num();

	// Write data to gridfs
	if (iov.iov_len != mongoc_gridfs_file_writev(file, &iov, 1, 0))
	{
		if (mongoc_gridfs_file_error(file, &error))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
				*FString(__func__), __LINE__, *FString(error.message));
		}
		mongoc_gridfs_file_destroy(file);
		return false;
	}

	// Saves modifications to file to the MongoDB server
	if (!mongoc_gridfs_file_save(file))
	{
		mongoc_gridfs_file_error(file, &error);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
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
