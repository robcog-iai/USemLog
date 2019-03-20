// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterMongoC.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"

// Constr
FSLWorldStateWriterMongoC::FSLWorldStateWriterMongoC()
{
	bIsInit = false;
}

// Init constr
FSLWorldStateWriterMongoC::FSLWorldStateWriterMongoC(const FSLWorldStateWriterParams& InParams)
{
	bIsInit = false;
	FSLWorldStateWriterMongoC::Init(InParams);
}

// Destr
FSLWorldStateWriterMongoC::~FSLWorldStateWriterMongoC()
{
	FSLWorldStateWriterMongoC::Finish();
#if SL_WITH_LIBMONGO_C
	//Release our handles and clean up libmongoc
	mongoc_collection_destroy(collection);
	mongoc_database_destroy(database);
	mongoc_uri_destroy(uri);
	mongoc_client_destroy(client);
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}

// Init
void FSLWorldStateWriterMongoC::Init(const FSLWorldStateWriterParams& InParams)
{
	MinLinearDistanceSquared = InParams.LinearDistanceSquared;
	MinAngularDistance = InParams.AngularDistance;
	bIsInit = FSLWorldStateWriterMongoC::Connect(InParams.Location, InParams.EpisodeId, InParams.ServerIp, InParams.ServerPort);
}

// Finish
void FSLWorldStateWriterMongoC::Finish()
{
	if (bIsInit)
	{
		FSLWorldStateWriterMongoC::CreateIndexes();
		bIsInit = false;
	}
}

void FSLWorldStateWriterMongoC::Write(float Timestamp,
	TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
	TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
	TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
	bool bCheckAndRemoveInvalidEntities)
{
	// Avoid writing empty documents
	if (ActorEntities.Num() == 0 && ComponentEntities.Num() == 0 && SkeletalEntities.Num() == 0)
	{
		return;
	}

#if SL_WITH_LIBMONGO_C
	bson_t* ws_doc;
	bson_t entities_arr;
	bson_error_t error;

	uint32_t arr_idx = 0;

	// Document to store the data
	ws_doc = bson_new();

	// Add timestamp
	BSON_APPEND_DOUBLE(ws_doc, "timestamp", Timestamp);

	// Add entities to array
	BSON_APPEND_ARRAY_BEGIN(ws_doc, "entities", &entities_arr);

	FSLWorldStateWriterMongoC::AddActorEntities(ActorEntities, &entities_arr, arr_idx);
	FSLWorldStateWriterMongoC::AddComponentEntities(ComponentEntities, &entities_arr, arr_idx);
	FSLWorldStateWriterMongoC::AddSkeletalEntities(SkeletalEntities, &entities_arr, arr_idx);

	bson_append_array_end(ws_doc, &entities_arr);


	if (!mongoc_collection_insert_one(collection, ws_doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		bson_destroy(ws_doc);
	}

	// Clean up
	bson_destroy(ws_doc);

#endif //SL_WITH_LIBMONGO_C
}

// Connect to the database
bool FSLWorldStateWriterMongoC::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
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
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SL_" + EpisodeId)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Abort if we connect to an existing collection
	if (mongoc_database_has_collection(database, TCHAR_TO_UTF8(*EpisodeId), &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Collection %s already exists in database.."),
			TEXT(__FUNCTION__), __LINE__, *EpisodeId);
		//return false;
	}
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

	bson_destroy(server_ping_cmd);
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Create indexes from the logged data, usually called after logging
bool FSLWorldStateWriterMongoC::CreateIndexes()
{
	if (!bIsInit)
	{
		return false;
	}
#if SL_WITH_LIBMONGO_C
	bson_t index;
	char *index_name;
	bson_t* index_command;
	bson_error_t error;
	
	bson_init(&index);
	BSON_APPEND_INT32(&index, "timestamp", 1);

	index_name = mongoc_collection_keys_to_index_string(&index);

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
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

#if SL_WITH_LIBMONGO_C
// Add non skeletal actors to array
void FSLWorldStateWriterMongoC::AddActorEntities(TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
	bson_t* out_doc, uint32_t& idx)
{
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;

	// Iterate items
	for (auto Itr(ActorEntities.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Obj.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Obj->GetActorLocation();
			const FQuat CurrQuat = Itr->Obj->GetActorQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > MinLinearDistanceSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
				BSON_APPEND_DOCUMENT_BEGIN(out_doc, idx_key, &arr_obj);

				BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*Itr->Entity.Id));
				AddPoseChild(CurrLoc, CurrQuat, &arr_obj);

				bson_append_document_end(out_doc, &arr_obj);
				idx++;
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLEntitiesManager::GetInstance()->RemoveEntity(Itr->Obj.Get());
		}
	}
}

// Add non skeletal components to array
void FSLWorldStateWriterMongoC::AddComponentEntities(TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
	bson_t* out_doc, uint32_t& idx)
{
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;

	// Iterate items
	for (auto Itr(ComponentEntities.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Obj.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Obj->GetComponentLocation();
			const FQuat CurrQuat = Itr->Obj->GetComponentQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > MinLinearDistanceSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
				BSON_APPEND_DOCUMENT_BEGIN(out_doc, idx_key, &arr_obj);

				BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*Itr->Entity.Id));
				AddPoseChild(CurrLoc, CurrQuat, &arr_obj);

				uint32_t arr_jdx = 10;


				bson_append_document_end(out_doc, &arr_obj);
				idx++;
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLEntitiesManager::GetInstance()->RemoveEntity(Itr->Obj.Get());
		}
	}
}

// Add skeletal actors to array
void FSLWorldStateWriterMongoC::AddSkeletalEntities(TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
	bson_t* out_doc, uint32_t& idx)
{
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;

	// Iterate items
	for (auto Itr(SkeletalEntities.CreateIterator()); Itr; ++Itr)
	{
		// Check if the entity moved more than the threshold since the last logging
		const FVector CurrLoc = Itr->Obj->GetComponentLocation();
		const FQuat CurrQuat = Itr->Obj->GetComponentQuat();

		// Check if pointer is valid
		if (Itr->Obj.IsValid(/*false, true*/))
		{
			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > MinLinearDistanceSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				bson_uint32_to_string(idx, &idx_key, idx_str, sizeof idx_str);
				BSON_APPEND_DOCUMENT_BEGIN(out_doc, idx_key, &arr_obj);

				BSON_APPEND_UTF8(&arr_obj, "id", TCHAR_TO_UTF8(*Itr->Entity.Id));
				AddPoseChild(CurrLoc, CurrQuat, &arr_obj);

				// Add bones
				if (Itr->Obj->SkeletalMeshParent)
				{
					AddSkeletalBones(Itr->Obj->SkeletalMeshParent, &arr_obj);
				}

				bson_append_document_end(out_doc, &arr_obj);
				idx++;
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLEntitiesManager::GetInstance()->RemoveEntity(Itr->Obj.Get());
		}
	}
}

// Add skeletal bones to array
void FSLWorldStateWriterMongoC::AddSkeletalBones(USkeletalMeshComponent* SkelComp, bson_t* out_doc)
{
	bson_t bones_arr;
	bson_t arr_obj;
	char idx_str[16];
	const char *idx_key;
	uint32_t arr_idx = 0;

	// Add entities to array
	BSON_APPEND_ARRAY_BEGIN(out_doc, "bones", &bones_arr);

	TArray<FName> BoneNames;
	SkelComp->GetBoneNames(BoneNames);
	for (const auto& BoneName : BoneNames)
	{
		const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
		const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

		bson_uint32_to_string(arr_idx, &idx_key, idx_str, sizeof idx_str);
		BSON_APPEND_DOCUMENT_BEGIN(&bones_arr, idx_key, &arr_obj);

		BSON_APPEND_UTF8(&arr_obj, "name", TCHAR_TO_UTF8(*BoneName.ToString()));
		AddPoseChild(CurrLoc, CurrQuat, &arr_obj);

		bson_append_document_end(&bones_arr, &arr_obj);
		arr_idx++;
	}

	bson_append_array_end(out_doc, &bones_arr);
}

// Add pose to document
void FSLWorldStateWriterMongoC::AddPoseChild(const FVector& InLoc, const FQuat& InQuat, bson_t* out_doc)
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