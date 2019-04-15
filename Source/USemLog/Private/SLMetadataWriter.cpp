// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMetadataWriter.h"
#include "SLEntitiesManager.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"

// Default constructor
FSLMetadataWriter::FSLMetadataWriter()
{
	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
FSLMetadataWriter::~FSLMetadataWriter()
{
	Finish(true);

#if SL_WITH_LIBMONGO_C
	// Release our handles and clean up mongoc
	mongoc_collection_destroy(collection);
	mongoc_database_destroy(database);
	mongoc_uri_destroy(uri);
	mongoc_client_destroy(client);
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}

// Init writer
void FSLMetadataWriter::Init(const FSLEventWriterParams& WriterParams)
{
	if (!bIsInit)
	{
#if SL_WITH_LIBMONGO_C
		bIsInit = Connect(WriterParams.Location, WriterParams.EpisodeId, WriterParams.ServerIp, WriterParams.ServerPort);
		// Write the initial pose and properties of all the entities
		WriteEnvironmentMetadata();
#endif //SL_WITH_LIBMONGO_C
	}
}


// Write the environment metadata
void FSLMetadataWriter::Start()
{
	if (!bIsStarted && bIsInit)
	{
		bIsStarted = true;
	}
}

// Write events metadata
void FSLMetadataWriter::Finish(bool bForced)
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		// Write events related semantic data
		WriteEventsMetadata();

		// Create indexes on the data
		CreateIndexes();

		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Write the environment metadata
void FSLMetadataWriter::WriteEnvironmentMetadata()
{
#if SL_WITH_LIBMONGO_C
	bson_t* meta_doc;
	bson_t* env_doc;
	bson_error_t error;
	
	// Document to store the metadata
	meta_doc = bson_new();
	env_doc = bson_new();
	
	// Add the unique id of the object
	bson_oid_init(&oid, NULL);
	BSON_APPEND_OID(meta_doc, "_id", &oid);

	// Add entities to the environment doc
	AddEntities(env_doc);

	// Add the env doc to the meta doc
	BSON_APPEND_DOCUMENT(meta_doc, "env", env_doc);

	if (!mongoc_collection_insert_one(collection, meta_doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bson_destroy(meta_doc);
	}

	// Clean up
	bson_destroy(meta_doc);
	bson_destroy(env_doc);

#endif //SL_WITH_LIBMONGO_C
}

// Write the episode events metadata
void FSLMetadataWriter::WriteEventsMetadata()
{
#if SL_WITH_LIBMONGO_C
	bson_t* evs_doc = bson_new();
	bson_t* update_doc = NULL;
	bson_t* update_query = NULL;
	bson_error_t error;

	// Add the events to the bson document
	AddEvents(evs_doc);

	// Command to update the existing document
	update_query = BCON_NEW("_id", BCON_OID(&oid));

	// Add the images data to the update document
	update_doc = BCON_NEW("$set", BCON_DOCUMENT(evs_doc));

	if (!mongoc_collection_update_one(collection, update_query, update_doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	// Clean up
	bson_destroy(evs_doc);
	bson_destroy(update_query);


#endif //SL_WITH_LIBMONGO_C
}

// Connect to the database
bool FSLMetadataWriter::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
	const FString MetaCollName = EpisodeId + ".meta";

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
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s"),
			*FString(__func__), __LINE__, *Uri, *FString(error.message));
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client)
	{
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SL_" + MetaCollName)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Abort if we connect to an existing collection
	if (mongoc_database_has_collection(database, TCHAR_TO_UTF8(*MetaCollName), &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Collection %s already exists in database.."),
			*FString(__func__), __LINE__, *MetaCollName);
		//return false;
	}

	collection = mongoc_client_get_collection(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*MetaCollName));

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
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Create databased for faster lookups
bool FSLMetadataWriter::CreateIndexes()
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
	BSON_APPEND_INT32(&index, "env.entities.id", 1);
	index_name = mongoc_collection_keys_to_index_string(&index);

	bson_init(&index2);
	BSON_APPEND_INT32(&index2, "env.entities.class", 1);
	index_name2 = mongoc_collection_keys_to_index_string(&index2);

	bson_init(&index3);
	BSON_APPEND_INT32(&index3, "env.entities.bones.class", 1);
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
			*FString(__func__), __LINE__, *FString(error.message));
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

#if SL_WITH_LIBMONGO_C
void FSLMetadataWriter::AddEntities(bson_t* out_doc)
{
	bson_t arr;
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;
	uint32_t idx = 0;

	// Add entities to array
	BSON_APPEND_ARRAY_BEGIN(out_doc, "entities", &arr);

	// Iterate non skeletal semantic entities
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
	{
		const FSLEntity SemEntity = Pair.Value;

		if (Cast<ASkeletalMeshActor>(SemEntity.Obj) || Cast<USkeletalMeshComponent>(SemEntity.Obj))
		{
			continue;
		}

		// Start array doc
		bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&arr, idx_key, &arr_obj);

		BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*Pair.Value.Id));
		BSON_APPEND_UTF8(&arr_obj, "class", TCHAR_TO_UTF8(*Pair.Value.Class));

		FString ColorHex = FTags::GetValue(Pair.Value.Obj, "SemLog", "VisMask");
		if (!ColorHex.IsEmpty())
		{
			BSON_APPEND_UTF8(&arr_obj, "mask_hex", TCHAR_TO_UTF8(*ColorHex));
		}

		// Check if location data is available
		if (AActor* ObjAsAct = Cast<AActor>(SemEntity.Obj))
		{
			const FVector Loc = ObjAsAct->GetActorLocation();
			const FQuat Quat = ObjAsAct->GetActorQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}
		else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(SemEntity.Obj))
		{
			const FVector Loc = ObjAsSceneComp->GetComponentLocation();
			const FQuat Quat = ObjAsSceneComp->GetComponentQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}

		// Finish array doc
		bson_append_document_end(&arr, &arr_obj);
		idx++;
	}

	// Iterate skeletal semantic entities
	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSkeletalSemanticData())
	{
		USLSkeletalDataComponent* SkelDataComp = Pair.Value;
		USkeletalMeshComponent* SkMComp = SkelDataComp->SkeletalMeshParent;
		const FSLEntity OwnerSemData = SkelDataComp->OwnerSemanticData;
		UObject* SemOwner = SkelDataComp->SemanticOwner;

		bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&arr, idx_key, &arr_obj);

		BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*OwnerSemData.Id));
		BSON_APPEND_UTF8(&arr_obj, "class", TCHAR_TO_UTF8(*OwnerSemData.Class));

		// Add semantic owner (component or actor) location
		if (AActor* ObjAsAct = Cast<AActor>(SemOwner))
		{
			const FVector Loc = ObjAsAct->GetActorLocation();
			const FQuat Quat = ObjAsAct->GetActorQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}
		else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(SemOwner))
		{
			const FVector Loc = ObjAsSceneComp->GetComponentLocation();
			const FQuat Quat = ObjAsSceneComp->GetComponentQuat();
			AddPoseChild(Loc, Quat, &arr_obj);
		}

		// Check if the skeletal mesh is valid
		if (SkMComp)
		{
			bson_t bones_arr;
			bson_t bones_arr_obj;
			char jdx_str[16];
			const char *jdx_key;
			uint32_t jdx = 0;

			BSON_APPEND_ARRAY_BEGIN(&arr_obj, "bones", &bones_arr);
			// Create array of all the bones (with and without(empty strings) semantic data)
			for (const auto& BoneNameToDataPair : SkelDataComp->AllBonesData)
			{
				const FName BoneName = BoneNameToDataPair.Key;
				const FSLBoneData BoneData = BoneNameToDataPair.Value;
				const FVector BoneLoc = SkMComp->GetBoneLocation(BoneName);
				const FQuat BoneQuat = SkMComp->GetBoneQuaternion(BoneName);

				bson_uint32_to_string(jdx, &jdx_key, jdx_str, sizeof jdx_str);
				BSON_APPEND_DOCUMENT_BEGIN(&bones_arr, jdx_key, &bones_arr_obj);

				BSON_APPEND_UTF8(&bones_arr_obj, "name", TCHAR_TO_UTF8(*BoneName.ToString()));

				if (!BoneData.Class.IsEmpty())
				{
					BSON_APPEND_UTF8(&bones_arr_obj, "class", TCHAR_TO_UTF8(*BoneData.Class));

					if (!BoneData.MaskColorHex.IsEmpty())
					{
						BSON_APPEND_UTF8(&bones_arr_obj, "mask_hex", TCHAR_TO_UTF8(*BoneData.MaskColorHex));
					}
				}

				AddPoseChild(BoneLoc, BoneQuat, &bones_arr_obj);

				bson_append_document_end(&bones_arr, &bones_arr_obj);
				jdx++;
			}
			// Add the created array to the semantic item
			bson_append_array_end(&arr_obj, &bones_arr);
		}

		// Add the semantic item to the array
		bson_append_document_end(&arr, &arr_obj);
		idx++;
	}

	bson_append_array_end(out_doc, &arr);
}

// Add semantic events data to the bson document
void FSLMetadataWriter::AddEvents(bson_t* out_doc)
{
	bson_t arr;
	/*bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;
	uint32_t idx = 0;*/

	// Add entities to array
	BSON_APPEND_ARRAY_BEGIN(out_doc, "events", &arr);

	bson_append_array_end(out_doc, &arr);
}

// Add pose to document
void FSLMetadataWriter::AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* out_doc)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	bson_t child_obj_loc;
	bson_t child_obj_rot;

	BSON_APPEND_DOCUMENT_BEGIN(out_doc, "loc", &child_obj_loc);
	BSON_APPEND_DOUBLE(&child_obj_loc, "x", ROSLoc.X);
	BSON_APPEND_DOUBLE(&child_obj_loc, "y", ROSLoc.Y);
	BSON_APPEND_DOUBLE(&child_obj_loc, "z", ROSLoc.Z);
	bson_append_document_end(out_doc, &child_obj_loc);

	BSON_APPEND_DOCUMENT_BEGIN(out_doc, "rot", &child_obj_rot);
	BSON_APPEND_DOUBLE(&child_obj_rot, "x", ROSQuat.X);
	BSON_APPEND_DOUBLE(&child_obj_rot, "y", ROSQuat.Y);
	BSON_APPEND_DOUBLE(&child_obj_rot, "z", ROSQuat.Z);
	BSON_APPEND_DOUBLE(&child_obj_rot, "w", ROSQuat.W);
	bson_append_document_end(out_doc, &child_obj_rot);
}

#endif //SL_WITH_LIBMONGO_C