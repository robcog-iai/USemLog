// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Mongo/SLMongoManager.h"
#include "Individuals/SLIndividualManager.h"
#include "EngineUtils.h"

// Sets default values
ASLMongoManager::ASLMongoManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bConnectedToServer = false;
	bDatabaseSet = false;
	bCollectionSet = false;
}

// Called when the game starts or when spawned
void ASLMongoManager::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ASLMongoManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Connect to the server
bool ASLMongoManager::ConnectToServer(const FString& Host, uint16 Port, bool bWithCheck)
{
	if (bConnectedToServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Already connected.."), *FString(__func__), __LINE__);
		return true;
	}

#if SL_WITH_LIBMONGO_C
	// Required to initialize libmongoc's internals
	mongoc_init();

	// Stores any error that might appear during the connection
	bson_error_t error;

	// Create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + Host + TEXT(":") + FString::FromInt(Port);
	const char *uri_string = TCHAR_TO_UTF8(*Uri);
	uri = mongoc_uri_new_with_error(uri_string, &error);
	if (!uri)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client)
	{
		bConnectedToServer = false;
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, "MongoQA");

	if (bWithCheck)
	{
		// Check server. Ping the "admin" database
		bson_t* server_ping_cmd;
		server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
		if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
				*FString(__func__), __LINE__, *FString(error.message));
			bson_destroy(server_ping_cmd);
			bConnectedToServer = false;
			return false;
		}
		bson_destroy(server_ping_cmd);
	}
	
	//UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully connected to: %s"), *FString(__func__), __LINE__, *Uri);		
	bConnectedToServer = true;
	return true;
#else
	bConnectedToServer = false;
	UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo module is missing.."), *FString(__func__), __LINE__);
	return false;
#endif // SL_WITH_LIBMONGO_C
}

// Connect to the given database
bool ASLMongoManager::SetDatabase(const FString& InDBName, bool bWithCheck)
{
	if (!bConnectedToServer)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not connected to server, connect first.."), *FString(__func__), __LINE__);
		return false;
	}
#if SL_WITH_LIBMONGO_C	
	bson_error_t error;
	if (bWithCheck)
	{		
		char **database_list;
		bool bDBExists = false;
		database_list = mongoc_client_get_database_names_with_opts(client, NULL, &error);
		if (database_list)
		{
			for (int i = 0; database_list[i]; i++)
			{
				if (InDBName == FString(database_list[i]))
				{
					bDBExists = true;
					//UE_LOG(LogTemp, Log, TEXT("%s::%d Database %s found.."), *FString(__func__), __LINE__, *FString(database_list[i]));
					break;
				}
			}
			bson_strfreev(database_list);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
				*FString(__func__), __LINE__, *FString(error.message));
			bDatabaseSet = false;
			return false;
		}

		if (!bDBExists)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Database %s not found.."),
				*FString(__func__), __LINE__, *InDBName);
			bDatabaseSet = false;
			return false;
		}
	}

	// Set the database
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*InDBName));

	// Check for meta collection
	if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*(InDBName + ".meta")), &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Database %s has no meta collection, creating a dummy one, some queries will not work.."),
			*FString(__func__), __LINE__, *InDBName);
	}
	meta_collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*(InDBName + ".meta")));

	bDatabaseSet = true;
	return true;	
#else
	UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo module is missing.."), *FString(__func__), __LINE__);
	bDatabaseSet = false;
	return false;
#endif
}

// Connect to the given collection
bool ASLMongoManager::SetCollection(const FString& InCollectionName, bool bWithCheck)
{
	if (!bDatabaseSet)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Database is not set.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C
	if (bWithCheck)
	{
		bson_error_t error;
		if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*InCollectionName), &error))
		{			
			UE_LOG(LogTemp, Error, TEXT("%s::%d Collection %s.%s not found.."),
				*FString(__func__), __LINE__, *FString(mongoc_database_get_name(database)), *InCollectionName);
			bCollectionSet = false;
			return false;
		}
	}

	// Set collection
	collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*InCollectionName));	
	bCollectionSet = true;
	return true;
#else
	UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo module is missing.."), *FString(__func__), __LINE__);
	bCollectionSet = false;
	return false;
#endif
}

// Clear and disconnect from the database
void ASLMongoManager::Disconnect()
{
	bConnectedToServer = false;
	bDatabaseSet = false;
	bCollectionSet = false;

#if SL_WITH_LIBMONGO_C
	// Release handles and clean up libmongoc
	if (meta_collection)
	{
		mongoc_collection_destroy(meta_collection);
	}
	if (collection)
	{
		mongoc_collection_destroy(collection);
	}
	if (database)
	{
		mongoc_database_destroy(database);
	}
	if (uri)
	{
		mongoc_uri_destroy(uri);
	}
	if (client)
	{
		mongoc_client_destroy(client);
	}
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}


/* Entity queries*/
// Get the entity pose at the given timestamp
bool ASLMongoManager::GetEntityPoseAt(const FString& Id, float Timestamp,
	FTransform& OutTransform) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C	
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp", "{", "$lte", BCON_DOUBLE(Timestamp), "}",
				"entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(-1),
			"}",
		"}",
		"{",
			"$limit", BCON_INT32(1),
		"}",
		"{",
			"$unwind", BCON_UTF8("$entities"),
		"}",
		"{",
			"$match",
			"{",
				"entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"loc", BCON_UTF8("$entities.loc"),
				"rot", BCON_UTF8("$entities.rot"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;
	if (mongoc_cursor_next(cursor, &doc))
	{
		OutTransform = GetPose(doc);
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	// Check if any error occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}

// Get the entity trajectory
bool ASLMongoManager::GetEntityTrajectory(const FString& Id, float StartTime, float EndTime,
	TArray<FTransform>& OutTransforms, float DeltaT) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}
	
#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp", 
				"{", 
					"$lte", BCON_DOUBLE(EndTime), 
					"$gte", BCON_DOUBLE(StartTime), 
				"}",
			"}",
		"}",
			"{",
				"$unwind", BCON_UTF8("$entities"),
			"}",
		"{",
			"$match",
			"{",
				"entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		//"{",
		//	"$sort",
		//		"{",
		//			"timestamp", BCON_INT32(1),
		//		"}",
		//	"}",
		//"}"
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"loc", BCON_UTF8("$entities.loc"),
				"rot", BCON_UTF8("$entities.rot"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	double PrevTs = - BIG_NUMBER;
	while (mongoc_cursor_next(cursor, &doc))
	{
		double CurrTs = GetTs(doc);
		if (CurrTs - PrevTs > DeltaT)
		{
			OutTransforms.Emplace(GetPose(doc));
			PrevTs = CurrTs;
		}
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	// Check if any error occured
	if (mongoc_cursor_error(cursor, &error)) 
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}


/* Bone queries */
// Get the bone pose at the given timestamp
bool ASLMongoManager::GetBonePoseAt(const FString& Id, const FString& BoneName, float Timestamp,
	FTransform& OutTransform) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}
	
#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;
	
	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp", "{", "$lte", BCON_DOUBLE(Timestamp), "}",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(-1),
			"}",
		"}",
		"{",
			"$limit", BCON_INT32(1),
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_entities"),
		"}",
		"{",
			"$match",
			"{",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"skel_entities.bones", BCON_INT32(1), 
			"}",
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_entities.bones"),
		"}",
		"{",
			"$match",
			"{",
				"skel_entities.bones.name", BCON_UTF8(TCHAR_TO_ANSI(*BoneName)),
			"}",
		"}",
		
		"{",
			"$project",
			"{",
				"timestamp", BCON_INT32(1),
				"loc", BCON_UTF8("$skel_entities.bones.loc"),
				"rot", BCON_UTF8("$skel_entities.bones.rot"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	if (mongoc_cursor_next(cursor, &doc))
	{
		OutTransform = GetPose(doc);
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	// Check if any error occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;	
#endif
	return false;
}

// Get the bone trajectory
bool ASLMongoManager::GetBoneTrajectory(const FString& Id, const FString& BoneName, float StartTime, float EndTime,
	TArray<FTransform>& OutTransforms, float DeltaT) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp",
				"{",
					"$lte", BCON_DOUBLE(EndTime),
					"$gte", BCON_DOUBLE(StartTime),
				"}",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_entities"),
		"}",
		"{",
			"$match",
			"{",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"skel_entities.bones", BCON_INT32(1), 
			"}",
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_entities.bones"),
		"}",
		"{",
			"$match",
			"{",
				"skel_entities.bones.name", BCON_UTF8(TCHAR_TO_ANSI(*BoneName)),
			"}",
		"}",
		//"{",
		//	"$sort",
		//		"{",
		//			"timestamp", BCON_INT32(1),
		//		"}",
		//	"}",
		//"}"
		"{",
			"$project",
			"{",
				"timestamp", BCON_INT32(1),
				"loc", BCON_UTF8("$skel_entities.bones.loc"),
				"rot", BCON_UTF8("$skel_entities.bones.rot"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	double PrevTs = -BIG_NUMBER;
	while (mongoc_cursor_next(cursor, &doc))
	{
		double CurrTs = GetTs(doc);
		if (CurrTs - PrevTs > DeltaT)
		{
			OutTransforms.Emplace(GetPose(doc));
			PrevTs = CurrTs;
		}
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	// Check if any error occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}


/* Skeletal queries */
// Get the skeletal pose at the given timestamp
bool ASLMongoManager::GetSkeletalPoseAt(const FString& Id, float Timestamp,
	TPair<FTransform, TMap<FString, FTransform>>& OutSkeletalPose) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp", "{", "$lte", BCON_DOUBLE(Timestamp), "}",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(-1),
			"}",
		"}",
		"{",
			"$limit", BCON_INT32(1),
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_entities"),
		"}",
		"{",
			"$match",
			"{",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"bones", BCON_UTF8("$skel_entities.bones"),
				"loc", BCON_UTF8("$skel_entities.loc"),		// Explicit skeletal actor location and rotation
				"rot", BCON_UTF8("$skel_entities.rot"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;
		
	if (mongoc_cursor_next(cursor, &doc))
	{
		OutSkeletalPose.Key = GetPose(doc);

		bson_iter_t bones;		
		if  (bson_iter_init(&bones, doc) && bson_iter_find(&bones, "bones"))
		{
			bson_iter_t bone;			
			if (bson_iter_recurse(&bones, &bone))
			{
				FString BoneName;
				bson_iter_t value;
				while (bson_iter_next(&bone))
				{
					if (bson_iter_recurse(&bone, &value) && bson_iter_find(&value, "name"))
					{
						BoneName = FString(bson_iter_utf8(&value, NULL));
					}
					OutSkeletalPose.Value.Emplace(BoneName, GetPose(&bone));
				}
			}
		}
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;
	
	// Check if any error occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}

// Get the skeltal trajectory
bool ASLMongoManager::GetSkeletalTrajectory(const FString& Id, float StartTime, float EndTime,
	TArray<TPair<FTransform, TMap<FString, FTransform>>>& OutSkeletalPoses, float DeltaT) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d MongoQA NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp", 
				"{", 
					"$lte", BCON_DOUBLE(EndTime),
					"$gte", BCON_DOUBLE(StartTime),
				"}",
			"}",
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_entities"),
		"}",
		"{",
			"$match",
			"{",
				"skel_entities.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),
			"}",
		"}",
		//"{",
		//	"$sort",
		//		"{",
		//			"timestamp", BCON_INT32(1),
		//		"}",
		//	"}",
		//"}"
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"bones", BCON_UTF8("$skel_entities.bones"),
				"loc", BCON_UTF8("$skel_entities.loc"),   // Keep the skeletal actor location and rotation
				"rot", BCON_UTF8("$skel_entities.rot"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;
	
	double PrevTs = -BIG_NUMBER;
	while (mongoc_cursor_next(cursor, &doc))
	{
		double CurrTs = GetTs(doc);
		if (CurrTs - PrevTs > DeltaT)
		{
			bson_iter_t bones;
			if (bson_iter_init(&bones, doc) && bson_iter_find(&bones, "bones"))
			{
				TMap<FString, FTransform> BonePoses;
				FString BoneName;

				bson_iter_t bone;				
				if (bson_iter_recurse(&bones, &bone))
				{
					bson_iter_t value;
					while (bson_iter_next(&bone))
					{
						if (bson_iter_recurse(&bone, &value) && bson_iter_find(&value, "name"))
						{
							BoneName = FString(bson_iter_utf8(&value, NULL));
						}
						BonePoses.Emplace(BoneName, GetPose(&bone));
					}
				}
				OutSkeletalPoses.Emplace(MakeTuple(GetPose(doc), BonePoses));
			}
			PrevTs = CurrTs;
		}
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;
		
	// Check if any errors occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;	
}


/* World queries */
// Get the state of all the entities in the world at a given time
bool ASLMongoManager::GetWorldStateAt(float Timestamp, TMap<FString, FTransform>& OutEntityPoses,
	TMap<FString, TPair<FTransform, TMap<FString, FTransform>>>& OutSkeletalPoses) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d MongoQA NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

	double ExecBegin = FPlatformTime::Seconds();
	TArray<FString> EntityIds;
	TArray<FString> SkeletalIds;
	if (!GetAllIdsInTheWorld(EntityIds, SkeletalIds))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not read ids, aborting.."), *FString(__func__), __LINE__);
		return false;
	}

	for (const auto& Id : EntityIds)
	{
		FTransform EntityPose;
		if (GetEntityPoseAt(Id, Timestamp, EntityPose))
		{
			OutEntityPoses.Emplace(Id, EntityPose);
		}		
	}

	for (const auto& SkId : SkeletalIds)
	{
		TPair<FTransform, TMap<FString, FTransform>> SkelPose;
		if (GetSkeletalPoseAt(SkId, Timestamp, SkelPose))
		{
			OutSkeletalPoses.Emplace(SkId, SkelPose);
		}		
	}

	UE_LOG(LogTemp, Log, TEXT("%s::%d Query duration total=[%f] seconds..;"),
		*FString(__func__), __LINE__, FPlatformTime::Seconds() - ExecBegin);
	return OutEntityPoses.Num() > 0 || OutSkeletalPoses.Num() > 0;
}

// Get the state of all the entities in the world between the timestamps
bool ASLMongoManager::GetAllWorldStates(TArray<FSLMongoWorldStateFrame>& OutWorldStates) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d MongoQA NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}
	
#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	bson_t opts;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp",
				"{",
					"$exists", BCON_BOOL(true),
				"}",
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(1),
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"entities", BCON_UTF8("$entities"),
				"skel_entities", BCON_UTF8("$skel_entities"),				
			"}",
		"}",
		"]");

	// If the episode is very large the hard drive needs to be used to cache results
	bson_init(&opts);
	BSON_APPEND_BOOL(&opts, "allowDiskUse", true);
	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, &opts, NULL);

	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;
	while (mongoc_cursor_next(cursor, &doc))
	{
		bson_iter_t iter;
		if (bson_iter_init(&iter, doc))
		{
			FSLMongoWorldStateFrame Frame;

			bson_iter_t entities;
			bson_iter_t skeletals;

			if (bson_iter_find(&iter, "timestamp"))
			{
				Frame.Timestamp = bson_iter_double(&iter);
			}

			if (bson_iter_find(&iter, "entities") && bson_iter_recurse(&iter, &entities))
			{
				while (bson_iter_next(&entities))
				{
					FString EntityId;

					bson_iter_t entity_value;
					if (bson_iter_recurse(&entities, &entity_value) && bson_iter_find(&entity_value, "id"))
					{
						EntityId = FString(bson_iter_utf8(&entity_value, NULL));
					}
					Frame.EntityPoses.Emplace(EntityId, GetPose(&entities));
				}
			}

			if (bson_iter_find(&iter, "skel_entities") && bson_iter_recurse(&iter, &skeletals))
			{ 
				while (bson_iter_next(&skeletals))
				{
					FString SkeletalId;

					bson_iter_t skeletal_value;
					if (bson_iter_recurse(&skeletals, &skeletal_value) && bson_iter_find(&skeletal_value, "id"))
					{
						SkeletalId = FString(bson_iter_utf8(&skeletal_value, NULL));
					}

					bson_iter_t bones;
					if (bson_iter_recurse(&skeletals, &bones) && bson_iter_find(&bones, "bones"))
					{
						TMap<FString, FTransform> BonePoses;
						FString BoneName;
						
						bson_iter_t bone;
						if (bson_iter_recurse(&bones, &bone))
						{
							bson_iter_t bone_value;
							while (bson_iter_next(&bone))
							{
								if (bson_iter_recurse(&bone, &bone_value) && bson_iter_find(&bone_value, "name"))
								{
									BoneName = FString(bson_iter_utf8(&bone_value, NULL));
								}
								BonePoses.Emplace(BoneName, GetPose(&bone));
							}
						}
						Frame.SkeletalPoses.Emplace(SkeletalId, MakeTuple(GetPose(&skeletals), BonePoses));
					}					
				}
			}
			OutWorldStates.Emplace(Frame);
		}
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	// Make frames store independently all the world state data (fast skimming),
	// currently every frame stores the changes relative to the previous one
	// merge Frame[Idx] with Frame[Idx+1]
	if (OutWorldStates.Num() > 1)
	{
		for (int32 Idx = 0; Idx < OutWorldStates.Num() - 1; ++Idx)
		{
			if (OutWorldStates[Idx].Timestamp >= OutWorldStates[Idx + 1].Timestamp)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Frame[%ld].Timestamp >= Frame[%ld].Timestamp.. this should not happen.."), *FString(__FUNCTION__), __LINE__);
				continue;
			}

			// Check if entity is missing in the next frame, if so, add it
			for (const auto& EntityPosePair : OutWorldStates[Idx].EntityPoses)
			{
				if (!OutWorldStates[Idx + 1].EntityPoses.Contains(EntityPosePair.Key))
				{
					OutWorldStates[Idx + 1].EntityPoses.Emplace(EntityPosePair.Key, EntityPosePair.Value);
				}
			}

			// Check if skeletal entity is missing in the next frame, if so, add it
			for (const auto& SkeletalPosePair : OutWorldStates[Idx].SkeletalPoses)
			{
				if (!OutWorldStates[Idx + 1].SkeletalPoses.Contains(SkeletalPosePair.Key))
				{
					OutWorldStates[Idx + 1].SkeletalPoses.Emplace(SkeletalPosePair.Key, SkeletalPosePair.Value);
				}
			}
		}
	}

	// Check if any errors occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}


/* Gaze queries */
// Get the target and the origin pose at the timestamp
bool ASLMongoManager::GetGazePose(float Timestamp, FVector& OutTarget, FVector& OutOrigin) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d MongoQA NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp", "{", "$lte", BCON_DOUBLE(Timestamp), "}",
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(-1),
			"}",
		"}",
		"{",
			"$limit", BCON_INT32(1),
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"target", BCON_UTF8("$gaze.target"),
				"origin", BCON_UTF8("$gaze.origin"),
			"}",
		"}",
		"]");


	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;
	if (mongoc_cursor_next(cursor, &doc))
	{
		bson_iter_t iter;
		bson_iter_t value;

		if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "origin.x", &value))
		{
			OutOrigin.X = bson_iter_double(&value);
		}
		if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "origin.y", &value))
		{
			OutOrigin.Y = bson_iter_double(&value);
		}
		if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "origin.z", &value))
		{
			OutOrigin.Z = bson_iter_double(&value);
		}
		if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "target.x", &value))
		{
			OutTarget.X = bson_iter_double(&value);
		}
		if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "target.y", &value))
		{
			OutTarget.Y = bson_iter_double(&value);
		}
		if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "target.z", &value))
		{
			OutTarget.Z = bson_iter_double(&value);
		}
#if SL_WITH_ROS_CONVERSIONS
		FConversions::ROSToU(OutOrigin);
		FConversions::ROSToU(OutTarget);
#endif // SL_WITH_ROS_CONVERSIONS
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;
	// Check if any errors occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}

// Get the gaze poses between the timestamps
bool ASLMongoManager::GetGazeTrajectory(float StartTime, float EndTime,
	TArray<FVector>& OutTarget, TArray<FVector>& OutOrigin, float DeltaT) const
{
	if (!IsReady())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d MongoQA NOT ready, cannot query.."), *FString(__func__), __LINE__);
		return false;
	}

#if SL_WITH_LIBMONGO_C
	double ExecBegin = FPlatformTime::Seconds();
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
			"timestamp",
				"{",
					"$lte", BCON_DOUBLE(EndTime),
					"$gte", BCON_DOUBLE(StartTime),
				"}",
			"}",
		"}",
		//"{",
		//	"$sort",
		//		"{",
		//			"timestamp", BCON_INT32(1),
		//		"}",
		//	"}",
		//"}"
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"target", BCON_UTF8("$gaze.target"),
				"origin", BCON_UTF8("$gaze.origin"),
			"}",
		"}",
		"]");	

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;
	double PrevTs = -BIG_NUMBER;
	while (mongoc_cursor_next(cursor, &doc))
	{
		double CurrTs = GetTs(doc);
		if (CurrTs - PrevTs > DeltaT)
		{
			FVector Origin;
			FVector Target;

			bson_iter_t iter;
			bson_iter_t value;
			if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "origin.x", &value))
			{
				Origin.X = bson_iter_double(&value) * 100.f;				
			}
			if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "origin.y", &value))
			{
				Origin.Y = bson_iter_double(&value) * -100.f;
			}
			if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "origin.z", &value))
			{
				Origin.Z = bson_iter_double(&value) * 100.f;
			}
			if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "target.x", &value))
			{
				Target.X = bson_iter_double(&value) * 100.f;
				UE_LOG(LogTemp, Log, TEXT("%s::%d Gaze Target X=%f;"), *FString(__FUNCTION__), __LINE__, Target.X);
			}
			if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "target.y", &value))
			{
				Target.Y = bson_iter_double(&value) * -100.f;
			}
			if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "target.z", &value))
			{
				Target.Z = bson_iter_double(&value) * 100.f;
			}
			// TODO this accumulates 100.f multiplication for some reason
			//FConversions::ROSToU(Origin);
			//FConversions::ROSToU(Target);

			OutTarget.Emplace(Target);
			OutOrigin.Emplace(Origin);

			PrevTs = CurrTs;
		}
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;
	
	// Check if any errors occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);

	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
#endif
	return false;
}

/* Helpers */
// Get the unique identifiers of the world entities
bool ASLMongoManager::GetAllIdsInTheWorld(TArray<FString>& OutEntityIds, TArray<FString>& OutSkeletalIds) const
{
#if SL_WITH_LIBMONGO_C
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline",
		"[",
			"{",
				"$project",
				"{",
					"_id", BCON_INT32(0),
					"entities", BCON_INT32(1),
					"skel_entities", BCON_INT32(1),
				"}",
			"}",
		"]");

	cursor = mongoc_collection_aggregate(meta_collection,
		MONGOC_QUERY_NONE, pipeline, NULL, NULL);

	if (mongoc_cursor_next(cursor, &doc))
	{
		bson_iter_t iter;
		
		if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "entities"))
		{
			FString EntityId;

			bson_iter_t entities;			
			if (bson_iter_recurse(&iter, &entities))
			{
				bson_iter_t value;
				while (bson_iter_next(&entities))
				{
					if (bson_iter_recurse(&entities, &value) && bson_iter_find(&value, "id"))
					{
						EntityId = FString(bson_iter_utf8(&value, NULL));
						OutEntityIds.Add(EntityId);
					}					
				}
			}
		}

		if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "skel_entities"))
		{
			FString SkeletalId;
			bson_iter_t skeletal;
			if (bson_iter_recurse(&iter, &skeletal))
			{
				bson_iter_t value;
				while (bson_iter_next(&skeletal))
				{
					if (bson_iter_recurse(&skeletal, &value) && bson_iter_find(&value, "id"))
					{
						SkeletalId = FString(bson_iter_utf8(&value, NULL));
						OutSkeletalIds.Add(SkeletalId);
					}					
				}
			}
		}
	}

	// Check if any errors occured
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	return true;
#endif
	return false;
}

#if SL_WITH_LIBMONGO_C
// Get transform from doc with "loc" and "rot" fields
FTransform ASLMongoManager::GetPose(const bson_t* doc) const
{	
	FVector Loc;
	FQuat Quat;
	
	bson_iter_t iter;
	bson_iter_t value;		
	
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "loc.x", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Loc.X = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "loc.y", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Loc.Y = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "loc.z", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Loc.Z = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "rot.x", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Quat.X = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "rot.y", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Quat.Y = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "rot.z", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Quat.Z = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "rot.w", &value) && BSON_ITER_HOLDS_DOUBLE(&value))
	{
		Quat.W = bson_iter_double(&value);
	}

	Quat.Normalize();
#if SL_WITH_ROS_CONVERSIONS
	return FConversions::ROSToU(FTransform(Quat, Loc));
#else
	return FTransform(Quat, Loc);
#endif // SL_WITH_ROS_CONVERSIONS	
}

// Get the timestamp value from "timestamp" field
double ASLMongoManager::GetTs(const bson_t* doc) const
{
	bson_iter_t iter;
	if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "timestamp"))
	{
		return bson_iter_double(&iter);
	}
	return -1.f;
}

// Get transform from iter with "loc" and "rot" fields
FTransform ASLMongoManager::GetPose(const bson_iter_t* iter) const
{
	FVector Loc;
	FQuat Quat;

	bson_iter_t value;
	bson_iter_t sub_value;

	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "loc.x", &sub_value))
	{
		Loc.X = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "loc.y", &sub_value))
	{
		Loc.Y = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "loc.z", &sub_value))
	{
		Loc.Z = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "rot.x", &sub_value))
	{
		Quat.X = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "rot.y", &sub_value))
	{
		Quat.Y = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "rot.z", &sub_value))
	{
		Quat.Z = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "rot.w", &sub_value))
	{
		Quat.W = bson_iter_double(&sub_value);
	}

	Quat.Normalize();
#if SL_WITH_ROS_CONVERSIONS
	return FConversions::ROSToU(FTransform(Quat, Loc));
#else
	return FTransform(Quat, Loc);
#endif // SL_WITH_ROS_CONVERSIONS	
}
#endif // SL_WITH_LIBMONGO_C