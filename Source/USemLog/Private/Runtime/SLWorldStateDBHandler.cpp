// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Runtime/SLWorldStateDBHandler.h"
#include "Individuals/SLIndividualManager.h"

// UUtils
#if SL_WITH_ROS_CONVERSIONS
#include "Conversions.h"
#endif // SL_WITH_ROS_CONVERSIONS


/* DB Write Async Task */
#if SL_WITH_LIBMONGO_C
bool FSLWorldStateDBWriterAsyncTask::Init(mongoc_collection_t* in_collection, ASLIndividualManager* IndividualManager, float MinLinearDistance, float MinAngularDistance)
{
	if (!IndividualManager->IsLoaded())
	{
		return false;
	}

	mongo_collection = in_collection;
	MinLinDistSqr = MinLinearDistance * MinLinearDistance;
	MinAngDist = MinAngularDistance;

	return true;
}
#endif //SL_WITH_LIBMONGO_C	

// Do the db writing here
void FSLWorldStateDBWriterAsyncTask::DoWork()
{
	const double StartTime = FPlatformTime::Seconds();

#if SL_WITH_LIBMONGO_C
	bson_t* ws_doc;
	bson_error_t error;

	ws_doc = bson_new();
	BSON_APPEND_DOUBLE(ws_doc, "timestamp", Timestamp);

	if (!mongoc_collection_insert_one(mongo_collection, ws_doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bson_destroy(ws_doc);
	}

	// Clean up
	bson_destroy(ws_doc);
#endif //SL_WITH_LIBMONGO_C	

	double Duration = FPlatformTime::Seconds() - StartTime;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t Async work duration:\t%f (s)"), *FString(__FUNCTION__), __LINE__, Duration);
}


/* DB handler */
// Ctor
FSLWorldStateDBHandler::FSLWorldStateDBHandler()
{
	bIsCleared = true;
	bIsConnected = false;
	DBWriterTask = nullptr;
}

// Dtor
FSLWorldStateDBHandler::~FSLWorldStateDBHandler()
{
	Finish();
}

// Connect to the db and set up the async writer
bool FSLWorldStateDBHandler::Init(ASLIndividualManager* IndividualManager,
	const FSLWorldStateLoggerParams& InLoggerParameters,
	const FSLLoggerLocationParams& InLocationParameters,
	const FSLLoggerDBServerParams& InDBServerParameters)
{
	bIsCleared = false;

	if (!Connect(InLocationParameters.TaskId, InLocationParameters.EpisodeId, 
		InDBServerParameters.Ip, InDBServerParameters.Port,
		InLocationParameters.bOverwrite))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d World state writer DB handler could not connect to the database.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bIsConnected = true;

	if (DBWriterTask == nullptr)
	{
		DBWriterTask = new FAsyncTask<FSLWorldStateDBWriterAsyncTask>();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state async writer should be nullptr here.."), *FString(__FUNCTION__), __LINE__);
	}

#if SL_WITH_LIBMONGO_C
	if (!DBWriterTask->GetTask().Init(collection, IndividualManager, InLoggerParameters.MinLinearDistance, InLoggerParameters.MinAngularDistance))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d World state async writer could not be initialized.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
#else
	UE_LOG(LogTemp, Error, TEXT("%s::%d SL_WITH_LIBMONGO_C flag is 0, aborting.."),
		*FString(__func__), __LINE__);
	return false;
#endif //SL_WITH_LIBMONGO_C

	return true;
}

// Delegate first job to the async task
void FSLWorldStateDBHandler::FirstWrite(float Timestamp)
{
	PrevWriteCallTime = FPlatformTime::Seconds();
	DBWriterTask->GetTask().SetTimestamp(Timestamp);
	DBWriterTask->StartBackgroundTask();
}

// Delegate job to the async task (true if the previous job was done)
bool FSLWorldStateDBHandler::Write(float Timestamp)
{
	double CurrentTime = FPlatformTime::Seconds();
	const double DurationSincePrevCall = CurrentTime - PrevWriteCallTime;
	PrevWriteCallTime = CurrentTime;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Duration since previous call:\t%f (s)"),
		*FString(__func__), __LINE__, DurationSincePrevCall);

	if (DBWriterTask->IsDone())
	{
		DBWriterTask->GetTask().SetTimestamp(Timestamp);
		DBWriterTask->StartBackgroundTask();
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Current db write async task is not finished yet, trying again next update call.."), *FString(__func__), __LINE__, Timestamp);
		return false;
	}
}

// Disconnect from db, clear task
void FSLWorldStateDBHandler::Finish()
{
	if (bIsCleared)
	{
		return;
	}

	CreateIndexes();
	Disconnect();
	bIsConnected = false;

	if (DBWriterTask != nullptr)
	{
		if (DBWriterTask->IsDone())
		{
			delete DBWriterTask;
			DBWriterTask = nullptr;
		}
		else
		{
			if (DBWriterTask->WaitCompletionWithTimeout(0.5f))
			{
				delete DBWriterTask;
				DBWriterTask = nullptr;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Task not completed, and writer was killed.."), *FString(__FUNCTION__), __LINE__);
				delete DBWriterTask;
				DBWriterTask = nullptr;
			}
		}
	}

	bIsCleared = true;
}

// Connect to the db
bool FSLWorldStateDBHandler::Connect(const FString& DBName, const FString& CollName, const FString& ServerIp,
		uint16 ServerPort, bool bOverwrite)
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
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SL_WorldStateWriter_" + CollName)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Check if the collection already exists
	if (mongoc_database_has_collection(database, TCHAR_TO_UTF8(*CollName), &error))
	{
		if (bOverwrite)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d World state collection %s already exists, will be removed and overwritten.."),
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
			UE_LOG(LogTemp, Warning, TEXT("%s::%d World state collection %s already exists and should not be overwritten, skipping metadata logging.."),
				*FString(__func__), __LINE__, *CollName);
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Creating collection %s.%s .."),
			*FString(__func__), __LINE__, *DBName, *CollName);
	}

	collection = mongoc_client_get_collection(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*CollName));

	// Check server. Ping the "admin" database
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
	UE_LOG(LogTemp, Error, TEXT("%s::%d SL_WITH_LIBMONGO_C flag is 0, aborting.."),
		*FString(__func__), __LINE__);
	return false;
#endif //SL_WITH_LIBMONGO_C
}
	
void FSLWorldStateDBHandler::Disconnect() const
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
bool FSLWorldStateDBHandler::CreateIndexes() const
{
	if (!bIsConnected)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Not connected to the db, could not create indexes.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
#if SL_WITH_LIBMONGO_C

	bson_t* index_command;
	bson_error_t error;
	
	bson_t idx_ts;
	bson_init(&idx_ts);
	BSON_APPEND_INT32(&idx_ts, "timestamp", 1);
	char* idx_ts_chr = mongoc_collection_keys_to_index_string(&idx_ts);

	bson_t idx_entities_id;
	bson_init(&idx_entities_id);
	BSON_APPEND_INT32(&idx_entities_id, "entities.id", 1);
	char* idx_entities_id_chr = mongoc_collection_keys_to_index_string(&idx_entities_id);

	bson_t idx_skel_entities_id;
	bson_init(&idx_skel_entities_id);
	BSON_APPEND_INT32(&idx_skel_entities_id, "skel_entities.id", 1);
	char* idx_skel_entities_id_chr = mongoc_collection_keys_to_index_string(&idx_skel_entities_id);

	bson_t idx_skel_entities_bones_name;
	bson_init(&idx_skel_entities_bones_name);
	BSON_APPEND_INT32(&idx_skel_entities_bones_name, "skel_entities.bones.name", 1);
	char* idx_skel_entities_bones_name_chr = mongoc_collection_keys_to_index_string(&idx_skel_entities_bones_name);

	bson_t idx_skel_entities_bones_id;
	bson_init(&idx_skel_entities_bones_id);
	BSON_APPEND_INT32(&idx_skel_entities_bones_id, "skel_entities.bones.id", 1);
	char* skel_entities_bones_id_chr = mongoc_collection_keys_to_index_string(&idx_skel_entities_bones_id);

	bson_t idx_gaze_entity_id;
	bson_init(&idx_gaze_entity_id);
	BSON_APPEND_INT32(&idx_gaze_entity_id, "gaze.entity_id", 1);
	char* idx_gaze_entity_id_chr = mongoc_collection_keys_to_index_string(&idx_gaze_entity_id);


	index_command = BCON_NEW("createIndexes",
			BCON_UTF8(mongoc_collection_get_name(collection)),
			"indexes",
			"[",
				"{",
					"key",
					BCON_DOCUMENT(&idx_ts),
					"name",
					BCON_UTF8(idx_ts_chr),
					"unique",
					BCON_BOOL(true),
				"}",
				"{",
					"key",
					BCON_DOCUMENT(&idx_entities_id),
					"name",
					BCON_UTF8(idx_entities_id_chr),
					//"unique",
					//BCON_BOOL(false),
				"}",
				"{",
					"key",
					BCON_DOCUMENT(&idx_skel_entities_id),
					"name",
					BCON_UTF8(idx_skel_entities_id_chr),
					//"unique",
					//BCON_BOOL(false),
				"}",
				"{",
					"key",
					BCON_DOCUMENT(&idx_skel_entities_bones_name),
					"name",
					BCON_UTF8(idx_skel_entities_bones_name_chr),
					//"unique",
					//BCON_BOOL(false),
				"}",
				"{",
					"key",
					BCON_DOCUMENT(&idx_skel_entities_bones_id),
					"name",
					BCON_UTF8(skel_entities_bones_id_chr),
					//"unique",
					//BCON_BOOL(false),
				"}",
				"{",
					"key",
					BCON_DOCUMENT(&idx_gaze_entity_id),
					"name",
					BCON_UTF8(idx_gaze_entity_id_chr),
					//"unique",
					//BCON_BOOL(false),
				"}",
			"]");

	if (!mongoc_collection_write_command_with_opts(collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bson_destroy(index_command);
		bson_free(idx_ts_chr);
		return false;
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(idx_ts_chr);
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

