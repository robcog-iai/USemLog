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
	// Avoid writing emtpy documents
	if (ActorEntities.Num() == 0 && ComponentEntities.Num() == 0 && SkeletalEntities.Num() == 0)
	{
		return;
	}

#if SL_WITH_LIBMONGO_C
	bson_t* entities_doc;
	bson_error_t error;

	// Document to store the images data
	entities_doc = bson_new();

	// Add timestamp
	BSON_APPEND_DOUBLE(entities_doc, "timestamp", Timestamp);


	if (!mongoc_collection_insert_one(collection, entities_doc, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		bson_destroy(entities_doc);
	}

	// Clean up
	bson_destroy(entities_doc);

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

