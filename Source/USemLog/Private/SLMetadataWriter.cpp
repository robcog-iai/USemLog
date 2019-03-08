// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMetadataWriter.h"

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
#endif //SL_WITH_LIBMONGO_C
	}
}


// Write the environment metadata
void FSLMetadataWriter::Start()
{
	if (!bIsStarted && bIsInit)
	{
		WriteEnvironmentMetadata();

		bIsStarted = true;
	}
}

// Write events metadata
void FSLMetadataWriter::Finish(bool bForced)
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		WriteEventsMetadata();

#if SL_WITH_LIBMONGO_C
		CreateIndexes();
#endif //SL_WITH_LIBMONGO_C

		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Write the environment metadata
void FSLMetadataWriter::WriteEnvironmentMetadata()
{
#if SL_WITH_LIBMONGO_C
	UE_LOG(LogTemp, Error, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);

	bson_t* env_doc;
	bson_error_t error;
	bson_t reply;
	char *str;

	// Document to store the environment data
	env_doc = bson_new();

	AddEntities(env_doc);

	if (!mongoc_collection_insert_one(collection, env_doc, NULL, &reply, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			TEXT(__FUNCTION__), __LINE__, *FString(error.message));
		bson_destroy(&reply);
		bson_destroy(env_doc);
	}

	str = bson_as_json(&reply, NULL);

	UE_LOG(LogTemp, Error, TEXT("%s::%d Reply.: %s"),
		TEXT(__FUNCTION__), __LINE__, *FString(str));

	// Clean up
	bson_destroy(&reply);
	bson_destroy(env_doc);
	bson_free(str);

#endif //SL_WITH_LIBMONGO_C
}

// Write the episode events metadata
void FSLMetadataWriter::WriteEventsMetadata()
{
#if SL_WITH_LIBMONGO_C
	UE_LOG(LogTemp, Error, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
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
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SL_" + MetaCollName)));

	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));
	collection = mongoc_client_get_collection(client, TCHAR_TO_UTF8(*DBName), TCHAR_TO_UTF8(*MetaCollName));

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

	bson_t* index_command;
	bson_error_t error;

	bson_init(&index);
	BSON_APPEND_INT32(&index, "env.entities.id", 1);
	index_name = mongoc_collection_keys_to_index_string(&index);

	bson_init(&index2);
	BSON_APPEND_INT32(&index2, "env.entities.class", 1);
	index_name2 = mongoc_collection_keys_to_index_string(&index2);

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
	bson_free(index_name2);
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

#if SL_WITH_LIBMONGO_C
void FSLMetadataWriter::AddEntities(bson_t* out_doc)
{
	/*bson_t* entities_doc;
	bson_t entities_arr;*/

	BSON_APPEND_UTF8(out_doc, "hello", "world");
}
#endif //SL_WITH_LIBMONGO_C