// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Runtime/SLWorldStateDBHandler.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLBaseIndividual.h"

// UUtils
#if SL_WITH_ROS_CONVERSIONS
#include "Conversions.h"
#endif // SL_WITH_ROS_CONVERSIONS


/* DB Write Async Task */
// Init task
#if SL_WITH_LIBMONGO_C
bool FSLWorldStateDBWriterAsyncTask::Init(mongoc_collection_t* in_collection, ASLIndividualManager* Manager, float PoseTolerance)
{
	if (!Manager->IsLoaded())
	{
		return false;
	}

	IndividualManager = Manager;
	mongo_collection = in_collection;
	MinPoseDiff = PoseTolerance;
		
	WriteFunctionPtr = &FSLWorldStateDBWriterAsyncTask::FirstWrite;

	return true;
}

// Do the db writing here
void FSLWorldStateDBWriterAsyncTask::DoWork()
{
	const double StartTime = FPlatformTime::Seconds();

	int32 NumEntries = (this->*WriteFunctionPtr)();

	double Duration = FPlatformTime::Seconds() - StartTime;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t Async work (written %ld entries) duration:\t%f (s)"),
		*FString(__FUNCTION__), __LINE__, NumEntries, Duration);
}

// First write where all the individuals are written irregardresly of their previous position
int32 FSLWorldStateDBWriterAsyncTask::FirstWrite()
{
	// Count the number of entries written to the document (if 0, abort writing)
	int32 Num = 0;

#if SL_WITH_LIBMONGO_C
	bson_t* ws_doc;
	ws_doc = bson_new();

	AddTimestamp(ws_doc);
	Num += AddAllIndividuals(ws_doc);
	UE_LOG(LogTemp, Warning, TEXT("%s::%d NumEntries=%ld"), *FString(__FUNCTION__), __LINE__, Num);
	// Write only if there are any entries in the document
	if (Num > 0)
	{
		WriteDoc(ws_doc);
	}

	// Clean up
	bson_destroy(ws_doc);
#endif //SL_WITH_LIBMONGO_C	

	// Switch to normal write
	WriteFunctionPtr = &FSLWorldStateDBWriterAsyncTask::Write;

	return Num;
}

// Write only the indviduals that changed pose
int32 FSLWorldStateDBWriterAsyncTask::Write()
{
	// Count the number of entries written to the document (if 0, abort writing)
	int32 Num = 0;

#if SL_WITH_LIBMONGO_C
	bson_t* ws_doc;
	ws_doc = bson_new();

	AddTimestamp(ws_doc);
	Num += AddAllIndividualsThatMoved(ws_doc);
	UE_LOG(LogTemp, Warning, TEXT("%s::%d NumEntries=%ld"), *FString(__FUNCTION__), __LINE__, Num);
	// Write only if there are any entries in the document
	if (Num > 0)
	{
		WriteDoc(ws_doc);
	}

	// Clean up
	bson_destroy(ws_doc);
#endif //SL_WITH_LIBMONGO_C	

	// Switch to normal write
	WriteFunctionPtr = &FSLWorldStateDBWriterAsyncTask::Write;

	return Num;
}

// Add timestamp to the bson doc
void FSLWorldStateDBWriterAsyncTask::AddTimestamp(bson_t* doc)
{
	BSON_APPEND_DOUBLE(doc, "timestamp", Timestamp);
}

// Add all individuals (return the number of individuals added)
int32 FSLWorldStateDBWriterAsyncTask::AddAllIndividuals(bson_t* doc)
{
	int32 Num = 0;
	bson_t individuals_arr;
	uint32_t arr_idx = 0;

	BSON_APPEND_ARRAY_BEGIN(doc, "individuals", &individuals_arr);
	for (const auto& Individual : IndividualManager->GetIndividuals())
	{
		Individual->UpdateCachedPose(0.0);
		// TODO workaround
		Individual->SetHasMovedFlag(true);
		
		
//#if SL_WITH_ROS_CONVERSIONS
//		FTransform Pose = FConversions::UToROS(Individual->GetCachedPose());
//#else
//		FTransform Pose = Individual->GetCachedPose();
//#endif // SL_WITH_ROS_CONVERSIONS

		bson_t individual_obj;
		char idx_str[16];
		const char* idx_key;

		bson_uint32_to_string(arr_idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&individuals_arr, idx_key, &individual_obj);
			// Id
			BSON_APPEND_UTF8(&individual_obj, "id", TCHAR_TO_UTF8(*Individual->GetIdValue()));			
			// Pose
			AddPose(Individual->GetCachedPose(), &individual_obj);
			//{
			//	bson_t child_obj_loc;
			//	bson_t child_obj_rot;

			//	BSON_APPEND_DOCUMENT_BEGIN(&individual_obj, "loc", &child_obj_loc);
			//	BSON_APPEND_DOUBLE(&child_obj_loc, "x", Pose.GetLocation().X);
			//	BSON_APPEND_DOUBLE(&child_obj_loc, "y", Pose.GetLocation().Y);
			//	BSON_APPEND_DOUBLE(&child_obj_loc, "z", Pose.GetLocation().Z);
			//	bson_append_document_end(&individual_obj, &child_obj_loc);

			//	BSON_APPEND_DOCUMENT_BEGIN(&individual_obj, "quat", &child_obj_rot);
			//	BSON_APPEND_DOUBLE(&child_obj_rot, "x", Pose.GetRotation().X);
			//	BSON_APPEND_DOUBLE(&child_obj_rot, "y", Pose.GetRotation().Y);
			//	BSON_APPEND_DOUBLE(&child_obj_rot, "z", Pose.GetRotation().Z);
			//	BSON_APPEND_DOUBLE(&child_obj_rot, "w", Pose.GetRotation().W);
			//	bson_append_document_end(&individual_obj, &child_obj_rot);
			//}
		bson_append_document_end(&individuals_arr, &individual_obj);
		arr_idx++;
		Num++;
	}
	bson_append_array_end(doc, &individuals_arr);
	return Num;
}

// Add only the individuals that moved (return the number of individuals added)
int32 FSLWorldStateDBWriterAsyncTask::AddAllIndividualsThatMoved(bson_t* doc)
{
	int32 Num = 0;

	bson_t individuals_arr;
	uint32_t arr_idx = 0;

	BSON_APPEND_ARRAY_BEGIN(doc, "individuals", &individuals_arr);
	for (const auto& Individual : IndividualManager->GetIndividuals())
	{
		if (Individual->UpdateCachedPose(MinPoseDiff))
		{
			// TODO workaround
			Individual->SetHasMovedFlag(true);

			bson_t individual_obj;
			char idx_str[16];
			const char* idx_key;

			bson_uint32_to_string(arr_idx, &idx_key, idx_str, sizeof idx_str);
			BSON_APPEND_DOCUMENT_BEGIN(&individuals_arr, idx_key, &individual_obj);
				// Id
				BSON_APPEND_UTF8(&individual_obj, "id", TCHAR_TO_UTF8(*Individual->GetIdValue()));
				// Pose
				AddPose(Individual->GetCachedPose(), &individual_obj);
			bson_append_document_end(&individuals_arr, &individual_obj);
			arr_idx++;
			Num++;
		}
	}
	bson_append_array_end(doc, &individuals_arr);
	return Num;
}

// Add skeletal individuals (return the number of individuals added)
int32 FSLWorldStateDBWriterAsyncTask::AddSkeletalIndividals(bson_t* doc)
{
	return 0;
}

// Add robot individuals (return the number of individuals added)
int32 FSLWorldStateDBWriterAsyncTask::AddRobotIndividuals(bson_t* doc)
{
	return 0;
}

// Add pose document
void FSLWorldStateDBWriterAsyncTask::AddPose(FTransform Pose, bson_t* doc)
{
#if SL_WITH_ROS_CONVERSIONS
	FConversions::UToROS(Pose);
#endif // SL_WITH_ROS_CONVERSIONS

	bson_t child_obj_loc;
	bson_t child_obj_rot;

	BSON_APPEND_DOCUMENT_BEGIN(doc, "loc", &child_obj_loc);
	BSON_APPEND_DOUBLE(&child_obj_loc, "x", Pose.GetLocation().X);
	BSON_APPEND_DOUBLE(&child_obj_loc, "y", Pose.GetLocation().Y);
	BSON_APPEND_DOUBLE(&child_obj_loc, "z", Pose.GetLocation().Z);
	bson_append_document_end(doc, &child_obj_loc);

	BSON_APPEND_DOCUMENT_BEGIN(doc, "quat", &child_obj_rot);
	BSON_APPEND_DOUBLE(&child_obj_rot, "x", Pose.GetRotation().X);
	BSON_APPEND_DOUBLE(&child_obj_rot, "y", Pose.GetRotation().Y);
	BSON_APPEND_DOUBLE(&child_obj_rot, "z", Pose.GetRotation().Z);
	BSON_APPEND_DOUBLE(&child_obj_rot, "w", Pose.GetRotation().W);
	bson_append_document_end(doc, &child_obj_rot);
}

// Write the bson doc to the collection
bool FSLWorldStateDBWriterAsyncTask::WriteDoc(bson_t* doc)
{
	bson_error_t error;
	if (!mongoc_collection_insert_one(mongo_collection, doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}
	return true;
}
#endif //SL_WITH_LIBMONGO_C	



/* DB Handler */
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
	if (!DBWriterTask->GetTask().Init(collection, IndividualManager, InLoggerParameters.PoseTolerance))
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

	bson_t idx_individuals_id;
	bson_init(&idx_individuals_id);
	BSON_APPEND_INT32(&idx_individuals_id, "individuals.id", 1);
	char* idx_individuals_id_chr = mongoc_collection_keys_to_index_string(&idx_individuals_id);

	//bson_t idx_entities_id;
	//bson_init(&idx_entities_id);
	//BSON_APPEND_INT32(&idx_entities_id, "entities.id", 1);
	//char* idx_entities_id_chr = mongoc_collection_keys_to_index_string(&idx_entities_id);

	//bson_t idx_skel_entities_id;
	//bson_init(&idx_skel_entities_id);
	//BSON_APPEND_INT32(&idx_skel_entities_id, "skel_entities.id", 1);
	//char* idx_skel_entities_id_chr = mongoc_collection_keys_to_index_string(&idx_skel_entities_id);

	//bson_t idx_skel_entities_bones_name;
	//bson_init(&idx_skel_entities_bones_name);
	//BSON_APPEND_INT32(&idx_skel_entities_bones_name, "skel_entities.bones.name", 1);
	//char* idx_skel_entities_bones_name_chr = mongoc_collection_keys_to_index_string(&idx_skel_entities_bones_name);

	//bson_t idx_skel_entities_bones_id;
	//bson_init(&idx_skel_entities_bones_id);
	//BSON_APPEND_INT32(&idx_skel_entities_bones_id, "skel_entities.bones.id", 1);
	//char* skel_entities_bones_id_chr = mongoc_collection_keys_to_index_string(&idx_skel_entities_bones_id);

	//bson_t idx_gaze_entity_id;
	//bson_init(&idx_gaze_entity_id);
	//BSON_APPEND_INT32(&idx_gaze_entity_id, "gaze.entity_id", 1);
	//char* idx_gaze_entity_id_chr = mongoc_collection_keys_to_index_string(&idx_gaze_entity_id);


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
					BCON_DOCUMENT(&idx_individuals_id),
					"name",
					BCON_UTF8(idx_individuals_id_chr),
					//"unique",
					//BCON_BOOL(false),
				"}",
				//"{",
				//	"key",
				//	BCON_DOCUMENT(&idx_entities_id),
				//	"name",
				//	BCON_UTF8(idx_entities_id_chr),
				//	//"unique",
				//	//BCON_BOOL(false),
				//"}",
				//"{",
				//	"key",
				//	BCON_DOCUMENT(&idx_skel_entities_id),
				//	"name",
				//	BCON_UTF8(idx_skel_entities_id_chr),
				//	//"unique",
				//	//BCON_BOOL(false),
				//"}",
				//"{",
				//	"key",
				//	BCON_DOCUMENT(&idx_skel_entities_bones_name),
				//	"name",
				//	BCON_UTF8(idx_skel_entities_bones_name_chr),
				//	//"unique",
				//	//BCON_BOOL(false),
				//"}",
				//"{",
				//	"key",
				//	BCON_DOCUMENT(&idx_skel_entities_bones_id),
				//	"name",
				//	BCON_UTF8(skel_entities_bones_id_chr),
				//	//"unique",
				//	//BCON_BOOL(false),
				//"}",
				//"{",
				//	"key",
				//	BCON_DOCUMENT(&idx_gaze_entity_id),
				//	"name",
				//	BCON_UTF8(idx_gaze_entity_id_chr),
				//	//"unique",
				//	//BCON_BOOL(false),
				//"}",
			"]");

	bool bRetVal = true;
	if (!mongoc_collection_write_command_with_opts(collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bRetVal = false;
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(idx_ts_chr);
	bson_free(idx_individuals_id_chr);
	return bRetVal;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

