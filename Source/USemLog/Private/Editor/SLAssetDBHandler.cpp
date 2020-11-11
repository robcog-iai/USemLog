// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLAssetDBHandler.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "AssetRegistryModule.h"
#endif // WITH_EDITOR

// Ctor
FSLAssetDBHandler::FSLAssetDBHandler() {}

// Connect to the database
bool FSLAssetDBHandler::Connect(const FString& DBName, const FString& ServerIp,
	uint16 ServerPort, ESLAssetAction InAction, bool bOverwrite)
{
	Action = InAction;

	if(Action != ESLAssetAction::Upload && Action != ESLAssetAction::Download)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Wrong action type.."),
			*FString(__func__), __LINE__);
		return false;
	}	

	const FString CollName = DBName + ".assets";

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
	TaskId = DBName;
	
	// Check if the collection already exists
	if (Action == ESLAssetAction::Upload || Action == ESLAssetAction::MoveAndUpload)
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

				// Create a new collection
				bson_t noopt = BSON_INITIALIZER;

				collection = mongoc_database_create_collection(database, TCHAR_TO_UTF8(*CollName), &noopt, &error);

				if (collection == NULL)
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
						*FString(__func__), __LINE__, *FString(error.message));
					return false;
				}

				UE_LOG(LogTemp, Warning, TEXT("%s::%d Successfully overwrite collection %s.%s for uploading assets .."),
					*FString(__func__), __LINE__, *DBName, *CollName);
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
			// Create a new collection
			bson_t noopt = BSON_INITIALIZER;

			collection = mongoc_database_create_collection(database, TCHAR_TO_UTF8(*CollName), &noopt, &error);

			if (collection == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
					*FString(__func__), __LINE__, *FString(error.message));
				return false;
			}
			
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Successfully created collection %s.%s for uploading assets .."),
				*FString(__func__), __LINE__, *DBName, *CollName);
		}

		// Create a gridfs handle prefixed by fs */
		gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), "fs", &error);
		if (!gridfs)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
				*FString(__func__), __LINE__, *Uri, *FString(error.message));
			return false;
		}

		if (bOverwrite)
		{
			// TODO mongoc_gridfs_find deprecated
			//mongoc_gridfs_file_list_t *list;
			//mongoc_gridfs_file_t *file_to_delete;
			//const bson_t query = BSON_INITIALIZER;

			//list = mongoc_gridfs_find(gridfs, &query);

			//file_to_delete = mongoc_gridfs_file_list_next(list);
			//while (file_to_delete != NULL) {
			//	if (!mongoc_gridfs_file_remove(file_to_delete, &error))
			//	{
			//		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			//			*FString(__func__), __LINE__, *Uri, *FString(error.message));
			//		return false;
			//	}
			//	file_to_delete = mongoc_gridfs_file_list_next(list);
			//}
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d Successfully clear gridfs"), *FString(__func__), __LINE__);
		}
	}
	else if(Action == ESLAssetAction::Download)
	{
		// Download read/only
		if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*CollName), &error))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Asset collection %s does not exist, skipping download.."),
				*FString(__func__), __LINE__, *CollName);
			return false;
		}

		// Set collection
		collection = mongoc_client_get_collection(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*CollName));

		UE_LOG(LogTemp, Warning, TEXT("%s::%d Successfully connected to the collection %s.%s for uploading assets .."),
			*FString(__func__), __LINE__, *DBName, *CollName);

		// Create a gridfs handle prefixed by fs */
		gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), "fs", &error);
		if (!gridfs)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
				*FString(__func__), __LINE__, *Uri, *FString(error.message));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Wrong action type.."),
			*FString(__func__), __LINE__);
		return false;
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

// 
void FSLAssetDBHandler::Execute()
{
	if (Action == ESLAssetAction::Upload)
	{
		Upload();
	}
	else if (Action == ESLAssetAction::Download)
	{
		Download();
	}
}

// Move current level and referenced asset to specific folder and upload to GridFS
void FSLAssetDBHandler::Upload()
{	
	UploadAllFileToGridFS(TEXT("/Game/SemLogAssets/") + TaskId + TEXT("/"));
}

// Download level and asset from GridFS
void FSLAssetDBHandler::Download()
{
#if SL_WITH_LIBMONGO_C
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	TMap<FString, FString> Files;
	FString Path;
	FString FileId;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"files", "{", "$exists", BCON_BOOL(true), "}",
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

	// change to if to load only one document
	while (mongoc_cursor_next(cursor, &doc))
	{
		bson_iter_t iter;
		bson_iter_t levels;

		if (bson_iter_init(&iter, doc))
		{
			if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "files"))
			{
				bson_iter_t value;

				if (bson_iter_recurse(&iter, &levels))
				{
					while (bson_iter_next(&levels))
					{
						if (bson_iter_recurse(&levels, &value) && bson_iter_find(&value, "file_id"))
						{
							char id[25];
							bson_oid_to_string(bson_iter_oid(&value), id);
							FileId = UTF8_TO_TCHAR(id);
						}
						if (bson_iter_recurse(&levels, &value) && bson_iter_find(&value, "path"))
						{
							Path = FString(bson_iter_utf8(&value, NULL));
						}
						Files.Add(FileId, Path);
					}
				}
			}
		}
	}
	// if there is error
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Log, TEXT("Cursor Failure: %s"), *FString(error.message));
	}
	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);

	//Create folder and download file	
	for (auto& Elem : Files)
	{
		FString FilePath = Elem.Value;
		FilePath.RemoveFromStart(TEXT("/Content/"));
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + FilePath + TEXT("/")));
		DownloadFileFromGridFS(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + FilePath + TEXT("/")), Elem.Key);
	}
#endif
}

#if SL_WITH_LIBMONGO_C
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
#endif

void FSLAssetDBHandler::UploadAllFileToGridFS(const FString& Dir)
{
#if WITH_EDITOR
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AllAsset;
	AssetRegistryModule.Get().GetAssetsByPath(FName(*Dir), AllAsset, true, false);
	TMap<FString, FString> Files;

	for (FAssetData Data : AllAsset)
	{
		FString Path = Data.PackagePath.ToString();
		Path.RemoveFromStart(TEXT("/Game/"));
		FString ContentDir = FPaths::ProjectContentDir() + Path + TEXT("/");

		FString Id;
		if (Data.AssetClass.ToString().Equals(TEXT("World")))
		{
			UploadFileToGridFS(ContentDir, Data.AssetName.ToString() + TEXT(".umap"), Id);
		}
		else
		{
			UploadFileToGridFS(ContentDir, Data.AssetName.ToString() + TEXT(".uasset"), Id);
		}
		Files.Add(Id, TEXT("/Content/") + Path);
		UE_LOG(LogTemp, Warning, TEXT("%s::%d fileid.: %s"),
			*FString(__func__), __LINE__, *Id);
	}

	WriteFilesToDocument(Files);
#endif // WITH_EDITOR
}

bool FSLAssetDBHandler::UploadFileToGridFS(const FString& Path, const FString& FileName, FString& InId)
{

#if SL_WITH_LIBMONGO_C

	bson_error_t error;
	bson_value_t file_id;
	mongoc_stream_t *file_stream;
	mongoc_gridfs_bucket_t *bucket;

	bucket = mongoc_gridfs_bucket_new(database, NULL, NULL, &error);
	if (!bucket)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	file_stream = mongoc_stream_file_new_for_path(TCHAR_TO_UTF8(*(Path + FileName)), O_RDONLY, 0);
	if (!file_stream)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: Can't open file stream"),
			*FString(__func__), __LINE__);
		return false;
	}

	bool result = mongoc_gridfs_bucket_upload_from_stream(
		bucket, TCHAR_TO_UTF8(*FileName), file_stream, NULL, &file_id, &error);

	if (!result)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	//const bson_oid_t id = file_id.value.v_oid;
	char id[25];
	bson_oid_to_string(&file_id.value.v_oid, id);
	InId = UTF8_TO_TCHAR(id);

	mongoc_stream_close(file_stream);
	mongoc_stream_destroy(file_stream);

	return true;

#else	
	return false;
#endif // SL_WITH_LIBMONGO_C
}

bool FSLAssetDBHandler::WriteFilesToDocument(TMap<FString, FString> Files)
{
#if SL_WITH_LIBMONGO_C

	bson_t     *document;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *files;
	bson_t *file_id;

	document = bson_new();
	files = bson_new();
	file_id = bson_new();

	bson_oid_init(&oid, NULL);
	BSON_APPEND_OID(document, "_id", &oid);

	BSON_APPEND_ARRAY_BEGIN(document, "files", files);
	for (auto& Elem : Files)
	{
		BSON_APPEND_DOCUMENT_BEGIN(files, "file_id", file_id);
		bson_oid_t id;
		char* id_string = TCHAR_TO_ANSI(*Elem.Key);
		bson_oid_init_from_string(&id, id_string);
		BSON_APPEND_OID(file_id, "file_id", &id);
		BSON_APPEND_UTF8(file_id, "path", TCHAR_TO_UTF8(*Elem.Value));
		bson_append_document_end(files, file_id);
	}
	bson_append_array_end(document, files);

	if (!mongoc_collection_insert_one(collection, document, NULL, NULL, &error)) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	bson_destroy(document);
	bson_destroy(files);

	return true;
#else
	return false;
#endif // SL_WITH_LIBMONGO_C
}

bool FSLAssetDBHandler::DownloadFileFromGridFS(const FString& Path, const FString& Id)
{
#if SL_WITH_LIBMONGO_C
	//bson_error_t error;
	//mongoc_gridfs_file_t *file;
	//const bson_value_t* file_id;
	//mongoc_stream_t *file_stream;
	//mongoc_gridfs_bucket_t *bucket;

	//bson_t *file_query;
	//bson_oid_t id;

	//file_query = bson_new();

	//char* id_string = TCHAR_TO_ANSI(*Id);
	//bson_oid_init_from_string(&id, id_string);
	//BSON_APPEND_OID(file_query, "_id", &id);

	//if (gridfs == nullptr)
	//	return false;

	// TODO deprecated
	//file = mongoc_gridfs_find_one(gridfs, file_query, &error);
	//if (!file)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
	//		*FString(__func__), __LINE__, *FString(error.message));
	//	return false;
	//}

	//file_id = mongoc_gridfs_file_get_id(file);

	//bucket = mongoc_gridfs_bucket_new(database, NULL, NULL, &error);
	//if (!bucket)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
	//		*FString(__func__), __LINE__, *FString(error.message));
	//	return false;
	//}

	//FString FileName = UTF8_TO_TCHAR(mongoc_gridfs_file_get_filename(file));
	//file_stream = mongoc_stream_file_new_for_path( TCHAR_TO_UTF8(*(Path+ FileName)), O_CREAT | O_RDWR, 0);
	//if (!file_stream)
	//{
	//	if ((Path + FileName).Len() > 245) {
	//		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: Can't open file stream, file path is too long"),
	//			*FString(__func__), __LINE__);
	//		return false;
	//	}
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: Can't open file stream"),
	//		*FString(__func__), __LINE__);
	//	return false;
	//}

	//bool result = mongoc_gridfs_bucket_download_to_stream(
	//	bucket, file_id, file_stream, &error);
	//if (!result) {
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
	//		*FString(__func__), __LINE__, *FString(error.message));
	//	return EXIT_FAILURE;
	//}

	//mongoc_stream_close(file_stream);
	//mongoc_stream_destroy(file_stream);

	return true;

#else
	return false;
#endif // SL_WITH_LIBMONGO_C
}