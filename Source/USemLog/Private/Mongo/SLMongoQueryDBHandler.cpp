// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Mongo/SLMongoQueryDBHandler.h"

#if SL_WITH_ROS_CONVERSIONS
#include "Conversions.h"
#endif // SL_WITH_ROS_CONVERSIONS

// Ctor
FSLMongoQueryDBHandler::FSLMongoQueryDBHandler()
{
	bConnected = false;
	bDatabaseSet = false;
	bCollectionSet = false;
}

// Dtor
FSLMongoQueryDBHandler::~FSLMongoQueryDBHandler()
{
	Disconnect();
}

// Connect to the server
bool FSLMongoQueryDBHandler::Connect(const FString& ServerIp, uint16 ServerPort)
{
	if (bConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler is already connected to the server.."), *FString(__func__), __LINE__);
		return true;
	}

	// Ping the db to double check the connection
	const bool bCheckConnection = true;

#if SL_WITH_LIBMONGO_C
	// Required to initialize libmongoc's internals
	mongoc_init();

	// Stores any error that might appear during the connection
	bson_error_t error;

	// Create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);
	const char* uri_string = TCHAR_TO_UTF8(*Uri);
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
		bConnected = false;
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create the mongo client.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, "MongoQA");

	if (bCheckConnection)
	{
		// Check server. Ping the "admin" database
		bson_t* server_ping_cmd;
		server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
		if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
				*FString(__func__), __LINE__, *FString(error.message));
			bson_destroy(server_ping_cmd);
			bConnected = false;
			return false;
		}
		bson_destroy(server_ping_cmd);
	}

	//UE_LOG(LogTemp, Log, TEXT("%s::%d Succesfully connected to: %s"), *FString(__func__), __LINE__, *Uri);		
	bConnected = true;
	return true;
#else
	UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo module is missing.."), *FString(__func__), __LINE__);
	return false;
#endif // SL_WITH_LIBMONGO_C
}

// Set database
bool FSLMongoQueryDBHandler::SetDatabase(const FString& InDBName)
{
	if (!bConnected)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not connected to server, connect first.."), *FString(__func__), __LINE__);
		return false;
	}

	// Double check that the database exists
	const bool bCheckDBExistence = true;

#if SL_WITH_LIBMONGO_C	
	bson_error_t error;

	// Iterate all db names to make sure it exists
	if (bCheckDBExistence)
	{
		char** database_list;
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

// Set collection
bool FSLMongoQueryDBHandler::SetCollection(const FString& InCollName)
{
	if (!bDatabaseSet)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Database is not set, cannot set collection without it.."), *FString(__func__), __LINE__);
		return false;
	}

	// Double check that the collection exists
	const bool bCheckCollExistence = true;

#if SL_WITH_LIBMONGO_C
	// Make sure the collection exits
	if (bCheckCollExistence)
	{
		bson_error_t error;
		if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*InCollName), &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Collection %s.%s not found.."),
				*FString(__func__), __LINE__, *FString(mongoc_database_get_name(database)), *InCollName);
			bCollectionSet = false;
			return false;
		}
	}

	// Set collection
	collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*InCollName));
	bCollectionSet = true;
	return true;
#else
	UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo module is missing.."), *FString(__func__), __LINE__);
	bCollectionSet = false;
	return false;
#endif
}

// Clear and disconnect from db
void FSLMongoQueryDBHandler::Disconnect()
{
	bConnected = false;
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

/* Queries */
// Get the pose of the individual at the given time
FTransform FSLMongoQueryDBHandler::GetIndividualPoseAt(const FString& Id, float Ts) const
{
	FTransform Pose;
	if (!IsReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d DB handler is not ready, make sure the server, database, and collection is set.."), *FString(__FUNCTION__), __LINE__);
		return Pose;
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
				"timestamp", "{", "$lte", BCON_DOUBLE(Ts), "}",
				"individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// yields faster results if we match against the id from the start
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(-1),							// required to get the last pose (no time penalty if the collection is indexed)
			"}",
		"}",
		"{",
			"$limit", BCON_INT32(1),
		"}",
		"{",
			"$unwind", BCON_UTF8("$individuals"),
		"}",
		"{",
			"$match",
			"{",
				"individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// match against the searched id in the unwinded array (has all individuals from the doc)
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"loc", BCON_UTF8("$individuals.loc"),
				"quat", BCON_UTF8("$individuals.quat"),
				"pose", BCON_UTF8("$individuals.pose"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	// Read cursor if no errors occured
	if (!mongoc_cursor_error(cursor, &error))
	{
		if (mongoc_cursor_next(cursor, &doc))
		{
			Pose = GetPose(doc);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
#endif
	return Pose;
}

// Get the poses of the individual between the given timestamps
TArray<FTransform> FSLMongoQueryDBHandler::GetIndividualTrajectory(const FString& Id, float StartTs, float EndTs, float DeltaT) const
{
	TArray<FTransform> Trajectory;
	if (!IsReady())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d DB handler is not ready, make sure the server, database, and collection is set.."), *FString(__FUNCTION__), __LINE__);
		return Trajectory;
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
					"$gte", BCON_DOUBLE(StartTs),
					"$lte", BCON_DOUBLE(EndTs),
				"}",
				"individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// yields faster results if we match against the id from the start
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(1),								// no time penalty if the collection is indexed
			"}",
		"}",
		"{",
			"$unwind", BCON_UTF8("$individuals"),
		"}",
		"{",
			"$match",
			"{",
				"individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// match against the searched id in the unwinded array (has all individuals from the doc)
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"loc", BCON_UTF8("$individuals.loc"),
				"quat", BCON_UTF8("$individuals.quat"),
				"pose", BCON_UTF8("$individuals.pose"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	// Read cursor if no errors occured
	if (!mongoc_cursor_error(cursor, &error))
	{
		if (DeltaT > 0.f)
		{
			double PrevTs = -BIG_NUMBER;
			while (mongoc_cursor_next(cursor, &doc))
			{
				double CurrTs = GetTs(doc);
				if (CurrTs - PrevTs > DeltaT)
				{
					Trajectory.Add(GetPose(doc));
					PrevTs = CurrTs;
				}
			}
		}
		else
		{
			while (mongoc_cursor_next(cursor, &doc))
			{
				Trajectory.Add(GetPose(doc));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds, Num=[%d]..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin, Trajectory.Num());
#endif
	if (Trajectory.Num() == 0)
	{
		Trajectory.Add(GetIndividualPoseAt(Id, StartTs));
	}
	return Trajectory;
}

// Get skeletal individual pose
TPair<FTransform, TMap<int32, FTransform>> FSLMongoQueryDBHandler::GetSkeletalIndividualPoseAt(const FString& Id, float Ts) const
{
	TPair<FTransform, TMap<int32, FTransform>> SkeletalPosePair;
	if (!IsReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d DB handler is not ready, make sure the server, database, and collection is set.."), *FString(__FUNCTION__), __LINE__);
		return SkeletalPosePair;
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
				"timestamp", "{", "$lte", BCON_DOUBLE(Ts), "}",
				"skel_individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// yields faster results if we match against the id from the start
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
			"$unwind", BCON_UTF8("$skel_individuals"),
		"}",
		"{",
			"$match",
			"{",
				"skel_individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// match against the searched id in the unwinded array (has all individuals from the doc)
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"bones", BCON_UTF8("$skel_individuals.bones"),		// bones data (index, loc, quat)
				"loc", BCON_UTF8("$skel_individuals.loc"),			// actor loc
				"quat", BCON_UTF8("$skel_individuals.quat"),		// actor quat
				"pose", BCON_UTF8("$skel_individuals.pose"),
			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;


	// Read cursor if no errors occured
	if (!mongoc_cursor_error(cursor, &error))
	{
		if (mongoc_cursor_next(cursor, &doc))
		{
			SkeletalPosePair.Key = GetPose(doc);

			// Get bones data
			bson_iter_t bones;
			if (bson_iter_init(&bones, doc) && bson_iter_find(&bones, "bones"))
			{
				bson_iter_t bone;
				if (bson_iter_recurse(&bones, &bone))
				{
					int32 BoneIndex;
					bson_iter_t value;
					while (bson_iter_next(&bone))
					{
						if (bson_iter_recurse(&bone, &value) && bson_iter_find(&value, "idx"))
						{
							BoneIndex = bson_iter_int32(&value);							
						}
						SkeletalPosePair.Value.Emplace(BoneIndex, GetPose(&bone));
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
#endif
	return SkeletalPosePair;
}

// Get skeletal individual trajectory
TArray<TPair<FTransform, TMap<int32, FTransform>>> FSLMongoQueryDBHandler::GetSkeletalIndividualTrajectory(const FString& Id, float StartTs, float EndTs, float DeltaT) const
{
	TArray<TPair<FTransform, TMap<int32, FTransform>>> SkeletalTrajectoryPair;
	if (!IsReady())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d DB handler is not ready, make sure the server, database, and collection is set.."), *FString(__FUNCTION__), __LINE__);
		return SkeletalTrajectoryPair;
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
					"$gte", BCON_DOUBLE(StartTs),
					"$lte", BCON_DOUBLE(EndTs),
				"}",
				"skel_individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// yields faster results if we match against the id from the start
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(1),								// if sort if right after match it barely adds any time penalty
			"}",
		"}",
		"{",
			"$unwind", BCON_UTF8("$skel_individuals"),
		"}",
		"{",
			"$match",
			"{",
				"skel_individuals.id", BCON_UTF8(TCHAR_TO_ANSI(*Id)),		// match against the searched id in the unwinded array (has all individuals from the doc)
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"bones", BCON_UTF8("$skel_individuals.bones"),		// bones data (index, loc, quat)
				"loc", BCON_UTF8("$skel_individuals.loc"),			// actor loc
				"quat", BCON_UTF8("$skel_individuals.quat"),		// actor quat
				"pose", BCON_UTF8("$skel_individuals.pose"),

			"}",
		"}",
		"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	// Read cursor if no errors occured
	if (!mongoc_cursor_error(cursor, &error))
	{
		if (DeltaT > 0.f)
		{
			double PrevTs = -BIG_NUMBER;
			while (mongoc_cursor_next(cursor, &doc))
			{
				double CurrTs = GetTs(doc);
				if (CurrTs - PrevTs > DeltaT)
				{
					TPair<FTransform, TMap<int32, FTransform>> SkeletalPosePair;
					SkeletalPosePair.Key = GetPose(doc);

					// Get bones data
					bson_iter_t bones;
					if (bson_iter_init(&bones, doc) && bson_iter_find(&bones, "bones"))
					{
						bson_iter_t bone;
						if (bson_iter_recurse(&bones, &bone))
						{
							int32 BoneIndex;
							bson_iter_t value;
							while (bson_iter_next(&bone))
							{
								if (bson_iter_recurse(&bone, &value) && bson_iter_find(&value, "idx"))
								{
									BoneIndex = bson_iter_int32(&value);
								}
								SkeletalPosePair.Value.Emplace(BoneIndex, GetPose(&bone));
							}
						}
					}
					SkeletalTrajectoryPair.Add(SkeletalPosePair);
					PrevTs = CurrTs;
				}
			}
		}
		else
		{
			while (mongoc_cursor_next(cursor, &doc))
			{
				TPair<FTransform, TMap<int32, FTransform>> SkeletalPosePair;
				SkeletalPosePair.Key = GetPose(doc);

				// Get bones data
				bson_iter_t bones;
				if (bson_iter_init(&bones, doc) && bson_iter_find(&bones, "bones"))
				{
					bson_iter_t bone;
					if (bson_iter_recurse(&bones, &bone))
					{
						int32 BoneIndex;
						bson_iter_t value;
						while (bson_iter_next(&bone))
						{
							if (bson_iter_recurse(&bone, &value) && bson_iter_find(&value, "idx"))
							{
								BoneIndex = bson_iter_int32(&value);
							}
							SkeletalPosePair.Value.Emplace(BoneIndex, GetPose(&bone));
						}
					}
				}
				SkeletalTrajectoryPair.Add(SkeletalPosePair);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor=[%f], total=[%f] seconds, Num=[%d]..;"),
		*FString(__func__), __LINE__, QueryDuration, CursorReadDuration, FPlatformTime::Seconds() - ExecBegin, SkeletalTrajectoryPair.Num());
#endif
	if (SkeletalTrajectoryPair.Num() == 0)
	{
		SkeletalTrajectoryPair.Add(GetSkeletalIndividualPoseAt(Id, StartTs));
	}
	return SkeletalTrajectoryPair;
}

// Get the whole episode data
TArray<TPair<float, TMap<FString, FTransform>>> FSLMongoQueryDBHandler::GetEpisodeData() const
{
	TArray<TPair<float, TMap<FString, FTransform>>> EpisodeData;
	if (!IsReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d DB handler is not ready, make sure the server, database, and collection is set.."), *FString(__FUNCTION__), __LINE__);
		return EpisodeData;
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
				"individuals", BCON_UTF8("$individuals"),
			"}",
		"}",
		"]");

	// If the episode is very large the hard drive needs to be used to cache results
	bson_init(&opts);
	BSON_APPEND_BOOL(&opts, "allowDiskUse", true);
	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, &opts, NULL);

	double QueryDuration = FPlatformTime::Seconds() - ExecBegin;

	int32 FrameIdx = 0;
	// Read cursor if no errors occured
	if (!mongoc_cursor_error(cursor, &error))
	{
		while (mongoc_cursor_next(cursor, &doc))
		{
			if (FrameIdx++ % 250 == 0) { UE_LOG(LogTemp, Log, TEXT(" mongo processing frame %d .."), FrameIdx++); }
			bson_iter_t frame_iter;
			if (bson_iter_init(&frame_iter, doc))
			{
				TMap<FString, FTransform> CurrIndividualsData;
				float CurrTs;
				
				if (bson_iter_find(&frame_iter, "timestamp"))
				{
					CurrTs = bson_iter_double(&frame_iter);
				}

				bson_iter_t individuals_iter;
				if (bson_iter_find(&frame_iter, "individuals") && bson_iter_recurse(&frame_iter, &individuals_iter))
				{
					while (bson_iter_next(&individuals_iter))
					{
						FString Id;
						bson_iter_t individual_val_iter;
						if (bson_iter_recurse(&individuals_iter, &individual_val_iter) && bson_iter_find(&individual_val_iter, "id"))
						{
							Id = FString(bson_iter_utf8(&individual_val_iter, NULL));
						}
						CurrIndividualsData.Emplace(Id, GetPose(&individuals_iter));
					}
				}
				EpisodeData.Emplace(CurrTs, CurrIndividualsData);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	double CursorReadDuration = FPlatformTime::Seconds() - ExecBegin - QueryDuration;

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: query=[%f], cursor(num=%d)=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, QueryDuration, EpisodeData.Num(), CursorReadDuration, FPlatformTime::Seconds() - ExecBegin);
#endif
	return EpisodeData;
}

// Get the whole episode data in an async thread
TArray<TPair<float, TMap<FString, FTransform>>> FSLMongoQueryDBHandler::GetEpisodeDataAsync() const
{
	// TODO
	return TArray<TPair<float, TMap<FString, FTransform>>>();
}

// Get the episode data at the given timestamp (frame)
TMap<FString, FTransform> FSLMongoQueryDBHandler::GetFrameData(float Ts)
{
	return TMap<FString, FTransform>();
}

/* Helpers */
#if SL_WITH_LIBMONGO_C
// Get the pose data from document
FTransform FSLMongoQueryDBHandler::GetPose(const bson_t* doc) const
{
	FVector Loc;
	FQuat Quat;

	bson_iter_t iter;
	bson_iter_t value;

	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "loc.x", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Loc.X = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "loc.y", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Loc.Y = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "loc.z", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Loc.Z = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "quat.x", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Quat.X = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "quat.y", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Quat.Y = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "quat.z", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Quat.Z = bson_iter_double(&value);
	}
	if (bson_iter_init(&iter, doc) && bson_iter_find_descendant(&iter, "quat.w", &value)/* && BSON_ITER_HOLDS_DOUBLE(&value)*/)
	{
		Quat.W = bson_iter_double(&value);
	}

	// using pose[x y z qx qy qz qw]
	//bson_iter_t iter;
	//const uint8_t *data = NULL;
	//uint32_t len = 0;
	//
	//if (bson_iter_init_find(&iter, doc, "pose"))
	//{
	//	bson_iter_array(&iter, &len, &data);
	//	bson_t * pose_arr = bson_new_from_data(data, len);
	//	bson_iter_t pose_iter;
	//	bson_iter_init(&pose_iter, pose_arr);
	//	
	//	bson_iter_find(&pose_iter, "0");
	//	Loc.X = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "1");
	//	Loc.Y = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "2");
	//	Loc.Z = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "3");
	//	Quat.X = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "4");
	//	Quat.Y = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "5");
	//	Quat.Z = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "6");
	//	Quat.W = bson_iter_double(&pose_iter);
	//}

	Quat.Normalize();
#if SL_WITH_ROS_CONVERSIONS
	return FConversions::ROSToU(FTransform(Quat, Loc));
#else
	return FTransform(Quat, Loc);
#endif // SL_WITH_ROS_CONVERSIONS	
}

// Get the pose data from iterator
FTransform FSLMongoQueryDBHandler::GetPose(const bson_iter_t* iter) const
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
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "quat.x", &sub_value))
	{
		Quat.X = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "quat.y", &sub_value))
	{
		Quat.Y = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "quat.z", &sub_value))
	{
		Quat.Z = bson_iter_double(&sub_value);
	}
	if (bson_iter_recurse(iter, &value) && bson_iter_find_descendant(&value, "quat.w", &sub_value))
	{
		Quat.W = bson_iter_double(&sub_value);
	}

	// using pose[x y z qx qy qz qw]
	//const uint8_t *data = NULL;
	//uint32_t len = 0;
	//bson_iter_t  value;
	//if (bson_iter_recurse(iter, &value) && bson_iter_find(&value, "pose"))
	//{
	//	bson_iter_array(&value, &len, &data);
	//	bson_t * pose_arr = bson_new_from_data(data, len);
	//	bson_iter_t pose_iter;
	//	bson_iter_init(&pose_iter, pose_arr);

	//	bson_iter_find(&pose_iter, "0");
	//	Loc.X = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "1");
	//	Loc.Y = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "2");
	//	Loc.Z = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "3");
	//	Quat.X = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "4");
	//	Quat.Y = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "5");
	//	Quat.Z = bson_iter_double(&pose_iter);
	//	bson_iter_find(&pose_iter, "6");
	//	Quat.W = bson_iter_double(&pose_iter);
	//}
	//

	Quat.Normalize();
#if SL_WITH_ROS_CONVERSIONS
	return FConversions::ROSToU(FTransform(Quat, Loc));
#else
	return FTransform(Quat, Loc);
#endif // SL_WITH_ROS_CONVERSIONS	
}

// Get the timestamp value from document (used for trajectory delta time comparison)
double FSLMongoQueryDBHandler::GetTs(const bson_t* doc) const
{
	bson_iter_t iter;
	if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "timestamp"))
	{
		return bson_iter_double(&iter);
	}
	return -1.f;
}
#endif // SL_WITH_LIBMONGO_C