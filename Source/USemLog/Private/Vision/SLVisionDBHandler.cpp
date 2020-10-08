// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionDBHandler.h"

// UUtils
#if SL_WITH_ROS_CONVERSIONS
#include "Conversions.h"
#endif // SL_WITH_ROS_CONVERSIONS


// Ctor
FSLVisionDBHandler::FSLVisionDBHandler() {}

// Connect to the database
bool FSLVisionDBHandler::Connect(const FString& DBName, const FString& CollName, const FString& ServerIp,
	uint16 ServerPort, bool bRemovePrevEntries)
{
	const FString VisCollName = CollName + ".vis";

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
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SLVIS_" + CollName)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Give a warning if the collection already exists or not
	if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*CollName), &error))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Collection %s does not exist, abort.."),
			*FString(__func__), __LINE__, *CollName);
		return false;
	}
	collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*CollName));

	if (mongoc_database_has_collection(database, TCHAR_TO_UTF8(*VisCollName), &error))
	{
		if (bRemovePrevEntries)
		{
			if (!mongoc_collection_drop(mongoc_database_get_collection(database, TCHAR_TO_UTF8(*VisCollName)), &error))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
					*FString(__func__), __LINE__, *FString(error.message));
			}
			if (!mongoc_collection_drop(mongoc_database_get_collection(database, TCHAR_TO_UTF8(*(VisCollName + ".chunks"))), &error))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
					*FString(__func__), __LINE__, *FString(error.message));
			}
			if (!mongoc_collection_drop(mongoc_database_get_collection(database, TCHAR_TO_UTF8(*(VisCollName + ".files"))), &error))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
					*FString(__func__), __LINE__, *FString(error.message));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Vis collection %s already exists and should not be overwritten, skipping vision logging.."),
				*FString(__func__), __LINE__, *VisCollName);
			return false;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("%s::%d Creating a new vis collection %s .."),
		*FString(__func__), __LINE__, *VisCollName);
	vis_collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*VisCollName));

	// Create a gridfs handle prefixed the vision collection
	gridfs = mongoc_client_get_gridfs(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*VisCollName), &error);
	if (!gridfs)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *Uri, *FString(error.message));
		return false;
	}

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

	// Remove previously added vision data
	if (bRemovePrevEntries)
	{
		//DropPreviousEntriesFromWorldColl_Legacy(DBName, CollName);
		DropPreviousEntries(DBName, CollName);
	}

	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Disconnect and clean db connection
void FSLVisionDBHandler::Disconnect() const
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
	if (vis_collection)
	{
		mongoc_collection_destroy(vis_collection);
	}
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}

// Create indexes on the inserted data
void FSLVisionDBHandler::CreateIndexes() const
{
#if SL_WITH_LIBMONGO_C
	bson_t* index_command;
	bson_error_t error;

	//bson_t index;
	//bson_init(&index);
	//BSON_APPEND_INT32(&index, "vision", 1);
	//char* index_str = mongoc_collection_keys_to_index_string(&index);
	bson_t idx_ts;
	bson_init(&idx_ts);
	BSON_APPEND_INT32(&idx_ts, "timestamp", 1);
	char* idx_ts_str = mongoc_collection_keys_to_index_string(&idx_ts);

	bson_t idx_id;
	bson_init(&idx_id);
	BSON_APPEND_INT32(&idx_id, "views.id", 1);
	char* idx_id_str = mongoc_collection_keys_to_index_string(&idx_id);

	bson_t idx_cls;
	bson_init(&idx_cls);
	BSON_APPEND_INT32(&idx_cls, "views.class", 1);
	char* idx_cls_str = mongoc_collection_keys_to_index_string(&idx_cls);

	bson_t idx_eid;
	bson_init(&idx_eid);
	BSON_APPEND_INT32(&idx_eid, "views.entities.id", 1);
	char* idx_eid_str = mongoc_collection_keys_to_index_string(&idx_eid);

	bson_t idx_ecls;
	bson_init(&idx_ecls);
	BSON_APPEND_INT32(&idx_ecls, "views.entities.class", 1);
	char* idx_ecls_str = mongoc_collection_keys_to_index_string(&idx_ecls);

	bson_t idx_skeid;
	bson_init(&idx_skeid);
	BSON_APPEND_INT32(&idx_skeid, "views.skel_entities.id", 1);
	char* idx_skeid_str = mongoc_collection_keys_to_index_string(&idx_skeid);

	bson_t idx_skecls;
	bson_init(&idx_skecls);
	BSON_APPEND_INT32(&idx_skecls, "views.skel_entities.class", 1);
	char* idx_skecls_str = mongoc_collection_keys_to_index_string(&idx_skecls);

	bson_t idx_skebcls;
	bson_init(&idx_skebcls);
	BSON_APPEND_INT32(&idx_skebcls, "views.skel_entities.bones.class", 1);
	char* idx_skebcls_str = mongoc_collection_keys_to_index_string(&idx_skebcls);

	index_command = BCON_NEW("createIndexes",
		BCON_UTF8(mongoc_collection_get_name(vis_collection)),
		"indexes",
		"[",
			"{",
				"key",
				BCON_DOCUMENT(&idx_ts),
				"name",
				BCON_UTF8(idx_ts_str),
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
			"{",
				"key",
				BCON_DOCUMENT(&idx_cls),
				"name",
				BCON_UTF8(idx_cls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_eid),
				"name",
				BCON_UTF8(idx_eid_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_ecls),
				"name",
				BCON_UTF8(idx_ecls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_skeid),
				"name",
				BCON_UTF8(idx_skeid_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_skecls),
				"name",
				BCON_UTF8(idx_skecls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_skebcls),
				"name",
				BCON_UTF8(idx_skebcls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
		"]");

	if (!mongoc_collection_write_command_with_opts(vis_collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(idx_ts_str);
	bson_free(idx_id_str);
	bson_free(idx_cls_str);
	bson_free(idx_eid_str);
	bson_free(idx_ecls_str);
	bson_free(idx_skeid_str);
	bson_free(idx_skecls_str);
	bson_free(idx_skebcls_str);
#endif //SL_WITH_LIBMONGO_C
}

// Get episode data from the database (UpdateRate = 0 means all the data)
bool FSLVisionDBHandler::GetEpisodeData(float UpdateRate, const TMap<ASkeletalMeshActor*,
	ASLVisionPoseableMeshActor*>& InSkelToPoseableMap,
	FSLVisionEpisode& OutEpisode)
{
	float CurrTs = 0.f;
	float PrevTs = -BIG_NUMBER; // this to make sure the first entry is loaded every time

#if SL_WITH_LIBMONGO_C
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

	bson_init(&opts);
	BSON_APPEND_BOOL(&opts, "allowDiskUse", true);

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, &opts, NULL);

	while (mongoc_cursor_next(cursor, &doc))
	{
		// Store the changes from the previous frame until this one
		FSLVisionFrame Frame;

		bson_iter_t doc_iter;
		if (bson_iter_init(&doc_iter, doc))
		{
			// Get the current timestamp
			if (bson_iter_find(&doc_iter, "timestamp"))
			{
				CurrTs = bson_iter_double(&doc_iter);
			}

			// Accumulate entity changes in the frame until the desired update rate is reached
			GetEntitiesData(&doc_iter, Frame.ActorPoses, Frame.VisionCameraPoses);

			// Accumulate skeletal entity changes in the frame until the desired update rate is reached
			GetSkeletalEntitiesData(&doc_iter, InSkelToPoseableMap, Frame.SkeletalPoses);

			// Check if the desired update rate is reached
			if (CurrTs - PrevTs >= UpdateRate)
			{
				// Update the previous timestamp
				PrevTs = CurrTs;

				// Add frame to episode and clear it for new data
				if (Frame.ActorPoses.Num() != 0 || Frame.SkeletalPoses.Num() != 0)
				{
					Frame.Timestamp = CurrTs;
					OutEpisode.AddFrame(Frame);
					Frame.Clear();
				}
			}
		}
	}

	// Check if any errors appeared while iterating the cursor
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to iterate all documents.. Err. %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);

	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Write current frame
void FSLVisionDBHandler::WriteFrame(const FSLVisionFrameData& Frame) const
{
#if SL_WITH_LIBMONGO_C
	// Document holding the frame data in bson format
	bson_t frame_doc;
	bson_init(&frame_doc);

	// Add resolution sub doc
	bson_t res_sub_doc;

	BSON_APPEND_DOCUMENT_BEGIN(&frame_doc, "res", &res_sub_doc);
	BSON_APPEND_INT32(&res_sub_doc, "x", Frame.Resolution.X);
	BSON_APPEND_INT32(&res_sub_doc, "y", Frame.Resolution.Y);
	bson_append_document_end(&frame_doc, &res_sub_doc);

	bson_t views_arr;
	bson_t views_arr_obj;

	bson_t entities_arr;
	bson_t entities_arr_obj;

	bson_t imgs_arr;
	bson_t imgs_arr_obj;

	bson_t bones_arr;
	bson_t bones_arr_obj;

	char i_str[16];
	const char *i_key;
	uint32_t i = 0;

	char j_str[16];
	const char *j_key;
	uint32_t j = 0;

	char k_str[16];
	const char *k_key;
	uint32_t k = 0;

	bson_oid_t file_oid;

	// Add timestamp
	BSON_APPEND_DOUBLE(&frame_doc, "timestamp", Frame.Timestamp);

	// Begin adding views data tot the 
	BSON_APPEND_ARRAY_BEGIN(&frame_doc, "views", &views_arr);

	// Iterate views (virtual cameras)
	for (const auto& ViewData : Frame.Views)
	{
		// Start array entry
		bson_uint32_to_string(i, &i_key, i_str, sizeof i_str);
		BSON_APPEND_DOCUMENT_BEGIN(&views_arr, i_key, &views_arr_obj);

		BSON_APPEND_UTF8(&views_arr_obj, "class", TCHAR_TO_UTF8(*ViewData.Class));
		BSON_APPEND_UTF8(&views_arr_obj, "id", TCHAR_TO_UTF8(*ViewData.Id));

		// Create the entities array
		j = 0;
		BSON_APPEND_ARRAY_BEGIN(&views_arr_obj, "entities", &entities_arr);
		for (const auto& Entity : ViewData.Entities)
		{
			bson_uint32_to_string(j, &j_key, j_str, sizeof j_str);
			BSON_APPEND_DOCUMENT_BEGIN(&entities_arr, j_key, &entities_arr_obj);

			BSON_APPEND_UTF8(&entities_arr_obj, "id", TCHAR_TO_UTF8(*Entity.Id));
			BSON_APPEND_UTF8(&entities_arr_obj, "class", TCHAR_TO_UTF8(*Entity.Class));
			BSON_APPEND_DOUBLE(&entities_arr_obj, "img_perc", Entity.ImagePercentage);
			BSON_APPEND_DOUBLE(&entities_arr_obj, "occl_perc", Entity.OcclusionPercentage);
			BSON_APPEND_BOOL(&entities_arr_obj, "clipped", Entity.bIsClipped);
		
			AddBBObj(Entity.MinBB, Entity.MaxBB, &entities_arr_obj);

			bson_append_document_end(&entities_arr, &entities_arr_obj);
			j++;
		}
		bson_append_array_end(&views_arr_obj, &entities_arr);

		// Create the skeletal entities array
		j = 0;
		BSON_APPEND_ARRAY_BEGIN(&views_arr_obj, "skel_entities", &entities_arr);
		for (const auto& SkelEntity : ViewData.SkelEntities)
		{
			bson_uint32_to_string(j, &j_key, j_str, sizeof j_str);
			BSON_APPEND_DOCUMENT_BEGIN(&entities_arr, j_key, &entities_arr_obj);

			BSON_APPEND_UTF8(&entities_arr_obj, "id", TCHAR_TO_UTF8(*SkelEntity.Id));
			BSON_APPEND_UTF8(&entities_arr_obj, "class", TCHAR_TO_UTF8(*SkelEntity.Class));
			BSON_APPEND_DOUBLE(&entities_arr_obj, "img_perc", SkelEntity.ImagePercentage);
			BSON_APPEND_DOUBLE(&entities_arr_obj, "occl_perc", SkelEntity.OcclusionPercentage);
			BSON_APPEND_BOOL(&entities_arr_obj, "clipped", SkelEntity.bIsClipped);
			AddBBObj(SkelEntity.MinBB, SkelEntity.MaxBB, &entities_arr_obj);

			// Create the bones array
			k = 0;
			BSON_APPEND_ARRAY_BEGIN(&entities_arr_obj, "bones", &bones_arr);
			for (const auto& Bone : SkelEntity.Bones)
			{
				bson_uint32_to_string(k, &k_key, k_str, sizeof k_str);
				BSON_APPEND_DOCUMENT_BEGIN(&bones_arr, k_key, &bones_arr_obj);

				BSON_APPEND_UTF8(&bones_arr_obj, "class", TCHAR_TO_UTF8(*Bone.Class));
				BSON_APPEND_DOUBLE(&bones_arr_obj, "img_perc", Bone.ImagePercentage);
				BSON_APPEND_DOUBLE(&bones_arr_obj, "occl_perc", Bone.OcclusionPercentage);
				BSON_APPEND_BOOL(&bones_arr_obj, "clipped", Bone.bIsClipped);

				AddBBObj(Bone.MinBB, Bone.MaxBB, &bones_arr_obj);

				bson_append_document_end(&bones_arr, &bones_arr_obj);
				k++;
			}
			bson_append_array_end(&entities_arr_obj, &bones_arr);

			bson_append_document_end(&entities_arr, &entities_arr_obj);
			j++;
		}
		bson_append_array_end(&views_arr_obj, &entities_arr);

		// Create the images array
		k = 0;
		BSON_APPEND_ARRAY_BEGIN(&views_arr_obj, "images", &imgs_arr);
		for (const auto& Img : ViewData.Images)
		{
			if (AddToGridFs(Img.Data, &file_oid))
			{
				bson_uint32_to_string(k, &k_key, k_str, sizeof k_str);
				BSON_APPEND_DOCUMENT_BEGIN(&imgs_arr, k_key, &imgs_arr_obj);

				BSON_APPEND_UTF8(&imgs_arr_obj, "type", TCHAR_TO_UTF8(*Img.Type));
				BSON_APPEND_OID(&imgs_arr_obj, "file_id", (const bson_oid_t*)&file_oid);

				bson_append_document_end(&imgs_arr, &imgs_arr_obj);
				k++;
			}
		}
		bson_append_array_end(&views_arr_obj, &imgs_arr);

		// End array entry
		bson_append_document_end(&views_arr, &views_arr_obj);
		i++;
	}
	bson_append_array_end(&frame_doc, &views_arr);

	// Update DB at the given timestamp with the document
	//WriteToWorldColl_Legacy(&frame_doc, Frame.Timestamp);
	WriteToVisionColl(&frame_doc);

	bson_destroy(&frame_doc);
#endif //SL_WITH_LIBMONGO_C
}

// Remove any previously added vision data from the database
void FSLVisionDBHandler::DropPreviousEntriesFromWorldColl_Legacy(const FString& DBName, const FString& CollName) const
{
#if SL_WITH_LIBMONGO_C
	// Remove all previous entries
	bson_t *selector = BCON_NEW("vision", "{", "$exists", BCON_BOOL(true), "}");
	bson_t *update = BCON_NEW("$unset", "{", "vision", BCON_BOOL(true), "}");
	bson_t reply;
	bson_error_t error;
	if (!mongoc_collection_update_many(collection, selector, update, NULL, &reply, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Check the number removed entries
	bson_iter_t iter;
	if (bson_iter_init_find(&iter, &reply, "modifiedCount") && BSON_ITER_HOLDS_INT(&iter))
	{
		int32 NumOfRemovedEntries = bson_iter_int32(&iter);
		if (NumOfRemovedEntries > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Removed %d previous entries.."),
				*FString(__func__), __LINE__, NumOfRemovedEntries);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No previous entries removed.."),
				*FString(__func__), __LINE__, NumOfRemovedEntries);
		}
	}

	// Remove gridfs collections
	if (!mongoc_collection_drop(mongoc_database_get_collection(database, TCHAR_TO_UTF8(*(CollName + ".chunks"))), &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop(mongoc_database_get_collection(database, TCHAR_TO_UTF8(*(CollName + ".files"))), &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not drop collection, err.:%s;"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Drop previous indexes to allow fast inserts
	TArray<const char*> Indexes;
	Indexes.Emplace("vision_1");
	Indexes.Emplace("vision.views.id_1");
	Indexes.Emplace("vision.views.class_1");
	Indexes.Emplace("vision.views.entities.id_1");
	Indexes.Emplace("vision.views.entities.class_1");
	Indexes.Emplace("vision.views.skel_entities.id_1");
	Indexes.Emplace("vision.views.skel_entities.class_1");
	Indexes.Emplace("vision.views.skel_entities.bones.class_1");

	for(const auto& Idx : Indexes)
	{
		if (!mongoc_collection_drop_index(collection, Idx, &error))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
				*FString(__func__), __LINE__, *FString(error.message));
		}
	}

	bson_destroy(selector);
	bson_destroy(update);
	bson_destroy(&reply);
#endif //SL_WITH_LIBMONGO_C
}

// Remove any previously added vision data from the database
void FSLVisionDBHandler::DropPreviousEntries(const FString& DBName, const FString& CollName) const
{

}

#if SL_WITH_LIBMONGO_C
// Get the entities data out of the bson iterator
bool FSLVisionDBHandler::GetEntitiesData(bson_iter_t* doc,
	TMap<AStaticMeshActor*, FTransform>& OutEntityPoses,
	TMap<ASLVirtualCameraView*, FTransform>& OutVirtualCameraPoses) const
{
	// Iterate entities
	if (bson_iter_find(doc, "entities"))
	{
		bson_iter_t child_iter;				// entities,
		bson_iter_t sub_child_iter;			// id, loc, rot
		bson_iter_t sub_sub_child_iter;		// x,y,z,w (entity)

		// Check if there are any entities
		if (bson_iter_recurse(doc, &child_iter))
		{
			FString Id;
			FVector Loc;
			FQuat Quat;

			while (bson_iter_next(&child_iter))
			{
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find(&sub_child_iter, "id"))
				{
					Id = FString(bson_iter_utf8(&sub_child_iter, NULL));
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "loc.x", &sub_sub_child_iter))
				{
					Loc.X = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "loc.y", &sub_sub_child_iter))
				{
					Loc.Y = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "loc.z", &sub_sub_child_iter))
				{
					Loc.Z = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.x", &sub_sub_child_iter))
				{
					Quat.X = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.y", &sub_sub_child_iter))
				{
					Quat.Y = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.z", &sub_sub_child_iter))
				{
					Quat.Z = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.w", &sub_sub_child_iter))
				{
					Quat.W = bson_iter_double(&sub_sub_child_iter);
				}

//				// Add entity
//				if (AStaticMeshActor* SMA = FSLEntitiesManager::GetInstance()->GetStaticMeshActor(Id))
//				{
//#if SL_WITH_ROS_CONVERSIONS
//					OutEntityPoses.Emplace(SMA, FConversions::ROSToU(FTransform(Quat, Loc)));
//#else
//					OutEntityPoses.Emplace(SMA, FTransform(Quat, Loc));
//#endif // SL_WITH_ROS_CONVERSIONS
//				}
//				else if (ASLVirtualCameraView* VCA = FSLEntitiesManager::GetInstance()->GetVisionCameraActor(Id))
//				{					
//#if SL_WITH_ROS_CONVERSIONS
//					OutVirtualCameraPoses.Emplace(VCA, FConversions::ROSToU(FTransform(Quat, Loc)));
//#else
//					OutVirtualCameraPoses.Emplace(VCA, FTransform(Quat, Loc));
//#endif // SL_WITH_ROS_CONVERSIONS
//				}
			}
		}
		return OutEntityPoses.Num() > 0;
	}
	else
	{
		return false;
	}
}

// Get the entities data out of the bson iterator, returns false if there are no entities
bool FSLVisionDBHandler::GetSkeletalEntitiesData(bson_iter_t* doc, 
	const TMap<ASkeletalMeshActor*, ASLVisionPoseableMeshActor*>& InSkelToPoseableMap,
	TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>>& OutSkeletalPoses) const
{
	// Iterate skeletal entities
	if (bson_iter_find(doc, "skel_entities"))
	{
		bson_iter_t child_iter;				// skel_entities
		bson_iter_t sub_child_iter;			// bones
		bson_iter_t sub_sub_child_iter;		// bones (array)

		if (bson_iter_recurse(doc, &child_iter))
		{
			FString Id;
			TMap<FName, FTransform> BonesMap;

			while (bson_iter_next(&child_iter))
			{
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find(&sub_child_iter, "id"))
				{
					Id = FString(bson_iter_utf8(&sub_child_iter, NULL));
				}

				if (bson_iter_recurse(&child_iter, &sub_sub_child_iter) && bson_iter_find(&sub_sub_child_iter, "bones"))
				{
					bson_iter_t bones_child;			// array  obj
					bson_iter_t bones_sub_child;		// name, loc, rot
					bson_iter_t bones_sub_sub_child;	// x, y , z, w

					FName BoneName;
					FVector Loc;
					FQuat Quat;

					if (bson_iter_recurse(&sub_sub_child_iter, &bones_child))
					{
						while (bson_iter_next(&bones_child))
						{
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find(&bones_sub_child, "name"))
							{
								BoneName = FName(bson_iter_utf8(&bones_sub_child, NULL));
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "loc.x", &bones_sub_sub_child))
							{
								Loc.X = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "loc.y", &bones_sub_sub_child))
							{
								Loc.Y = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "loc.z", &bones_sub_sub_child))
							{
								Loc.Z = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.x", &bones_sub_sub_child))
							{
								Quat.X = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.y", &bones_sub_sub_child))
							{
								Quat.Y = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.z", &bones_sub_sub_child))
							{
								Quat.Z = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.w", &bones_sub_sub_child))
							{
								Quat.W = bson_iter_double(&bones_sub_sub_child);
							}
#if SL_WITH_ROS_CONVERSIONS
							BonesMap.Add(BoneName, FConversions::ROSToU(FTransform(Quat, Loc)));
#else
							BonesMap.Add(BoneName, FTransform(Quat, Loc));
#endif // SL_WITH_ROS_CONVERSIONS
						}
					}
				}

				//// Add skeletal entity
				//if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor((Id)))
				//{
				//	if (ASLVisionPoseableMeshActor* const* PMA = InSkelToPoseableMap.Find(SkMA))
				//	{
				//		OutSkeletalPoses.Emplace(*PMA, BonesMap);
				//	}
				//	else
				//	{
				//		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find poseable mesh clone actor for %s, did you run the setup before?"),
				//			*FString(__func__), __LINE__, *SkMA->GetName());
				//	}
				//}
			}
		}
		return OutSkeletalPoses.Num() > 0;
	}
	return false;
}

// Save image to gridfs, get the file oid and return true if succeeded
bool FSLVisionDBHandler::AddToGridFs(const TArray<uint8>& InData, bson_oid_t* out_oid) const
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

// Write the bson doc containing the vision data to the entry corresponding to the timestamp
bool FSLVisionDBHandler::WriteToWorldColl_Legacy(bson_t* doc, float Timestamp) const
{
	// Return flag
	bool bSuccess = true;

	// Update the entry at the given timestamp
	bson_t* selector = BCON_NEW("timestamp", BCON_DOUBLE(Timestamp));
	bson_t* update = BCON_NEW("$set", "{", "vision", BCON_DOCUMENT(doc), "}");
	bson_t reply;
	bson_error_t error;
	if (!mongoc_collection_update_one(collection, selector, update, NULL, &reply, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Check if the entry was updated
	bson_iter_t iter;
	if (bson_iter_init_find(&iter, &reply, "modifiedCount") && BSON_ITER_HOLDS_INT(&iter))
	{
		int32 NumOfUpdatedEntries = bson_iter_int32(&iter);
		if (NumOfUpdatedEntries == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not update entry at timestamp %f"),
				*FString(__func__), __LINE__, Timestamp);
			bSuccess = false;
		}
		else if (NumOfUpdatedEntries > 1)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Updated more than one (%d) entry at timestamp %f, this should not happen.."),
				*FString(__func__), __LINE__, NumOfUpdatedEntries, Timestamp);
			bSuccess = false;
		}
		else if (NumOfUpdatedEntries == 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Succesfully updated entry at timestamp %f"),
				*FString(__func__), __LINE__, Timestamp);
			bSuccess = true;
		}
	}

	// Cleanup
	bson_destroy(selector);
	bson_destroy(update);
	bson_destroy(&reply);

	return bSuccess;
}

// Write the bson doc containing the vision data to the entry corresponding to the timestamp
bool FSLVisionDBHandler::WriteToVisionColl(bson_t* doc) const
{
	bson_error_t error;
	if (!mongoc_collection_insert_one(vis_collection, doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}
	return true;
}

// Add image bounding box to document
void FSLVisionDBHandler::AddBBObj(const FIntPoint& Min, const FIntPoint& Max, bson_t* doc) const
{
	bson_t bb;
	bson_t child_min_bb;
	bson_t child_max_bb;

	BSON_APPEND_DOCUMENT_BEGIN(doc, "img_bb", &bb);
		BSON_APPEND_DOCUMENT_BEGIN(&bb, "min", &child_min_bb);
			BSON_APPEND_INT32(&child_min_bb, "x", Min.X);
			BSON_APPEND_INT32(&child_min_bb, "y", Min.Y);
		bson_append_document_end(&bb, &child_min_bb);
		BSON_APPEND_DOCUMENT_BEGIN(&bb, "max", &child_max_bb);
			BSON_APPEND_INT32(&child_max_bb, "x", Max.X);
			BSON_APPEND_INT32(&child_max_bb, "y", Max.Y);
		bson_append_document_end(&bb, &child_max_bb);
	bson_append_document_end(doc, &bb);
}
#endif //SL_WITH_LIBMONGO_C
