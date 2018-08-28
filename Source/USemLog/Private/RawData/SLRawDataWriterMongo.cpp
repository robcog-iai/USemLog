// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "RawData/SLRawDataWriterMongo.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"
#if WITH_LIBMONGO
#include "mongoc.h"
#include "bson.h"
#include "string"
#endif //WITH_LIBMONGO

// Constr
FSLRawDataWriterMongo::FSLRawDataWriterMongo()
{
}

// Constr with Init
FSLRawDataWriterMongo::FSLRawDataWriterMongo(FSLRawDataAsyncWorker* InWorkerParent,
	const FString& InLogDB,
	const FString& InEpisodeId,
	const FString& InMongoIP,
	uint16 MongoPort)
{
	Init(InWorkerParent, InLogDB, InEpisodeId, InMongoIP, MongoPort);
}

// Destr
FSLRawDataWriterMongo::~FSLRawDataWriterMongo()
{
	// Disconnect mongo (do not need to disconnect,if disconnect, Play game for the seconde time will crash)
	/*mongoc_collection_destroy(collection);
	mongoc_database_destroy(database);
	mongoc_client_destroy(client);*/
	//mongoc_cleanup();
}

// Init
void FSLRawDataWriterMongo::Init(FSLRawDataAsyncWorker* InWorkerParent,
	const FString& InLogDB,
	const FString& InEpisodeId,
	const FString& InMongoIP,
	uint16 MongoPort)
{
	WorkerParent = InWorkerParent;
	bConnect = ConnectToMongo(InLogDB, InEpisodeId, InMongoIP, MongoPort);
}

// Called to write the data
void FSLRawDataWriterMongo::WriteData()
{
#if WITH_LIBMONGO
	if (bConnect)
	{
		// Bson root object
		bson_t* BsonRootObj;
		BsonRootObj = bson_new();

		// Bson Array of entities
		bson_t BsonEntitiesArr;
		bson_init(&BsonEntitiesArr);

		// Add actors data to the bson array
		FSLRawDataWriterMongo::AddActors(BsonEntitiesArr);

		// Add components data to the bson array
		FSLRawDataWriterMongo::AddComponents(BsonEntitiesArr);

		
		if (sizeof(BsonEntitiesArr) > 0)
		{
			// set timestamp
			BSON_APPEND_DOUBLE(BsonRootObj, "timestamp", WorkerParent->World->GetTimeSeconds());

			// add entity array to root oject
			BSON_APPEND_ARRAY(BsonRootObj, "entities", &BsonEntitiesArr);
		}
		
		// Write data in Mongo data base
		FSLRawDataWriterMongo::WriteToMongo(BsonRootObj,collection);

		bson_destroy(BsonRootObj);
	}
#endif //WITH_LIBMONGO
}

// Set the file handle for the logger
bool FSLRawDataWriterMongo::ConnectToMongo(const FString& InLogDB,
	const FString& InEpisodeId,
	const FString& InMongoIP,
	uint16 MongoPort)
{	
#if WITH_LIBMONGO
	// set the uri address
	FString Furi_str = TEXT("mongodb://")+ InMongoIP+TEXT(":") + FString::FromInt(MongoPort);
	
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Furi_str);
	//convert inputs FString to std::string
	std::string uri_str(TCHAR_TO_UTF8(*Furi_str));
	std::string database_name(TCHAR_TO_UTF8(*InLogDB));
	std::string collection_name(TCHAR_TO_UTF8(*InEpisodeId));

	

	//Required to initialize libmongoc's internals
	mongoc_init();
	
	//create a new client instance
	client = mongoc_client_new(uri_str.c_str());

	//Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, "Mongo_data");

	//Get a handle on the database and collection 
	database = mongoc_client_get_database(client, database_name.c_str());
	collection = mongoc_client_get_collection(client, database_name.c_str(), collection_name.c_str());

	//connect mongo database
	if (client) {
		
		UE_LOG(LogTemp, Warning, TEXT("Mongo Database has been connected"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Database connection failed"));
		return false;
	}
#else
	return false;
#endif //WITH_LIBMONGO	
}

#if WITH_LIBMONGO
// Add actors
void FSLRawDataWriterMongo::AddActors(bson_t& OutBsonEntitiesArr)
{
	// Iterate actors
	for (auto RawDataActItr(WorkerParent->RawDataActors.CreateIterator()); RawDataActItr; ++RawDataActItr)
	{
		// Check if pointer is valid
		if (RawDataActItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataActItr->Entity->GetActorLocation();
			const FQuat CurrQuat = RawDataActItr->Entity->GetActorQuat();
			if (FVector::DistSquared(CurrLoc, RawDataActItr->PrevLoc) > WorkerParent->DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataActItr->PrevLoc = CurrLoc;

				// Get current entry as Bson object
				bson_t BsonActorEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
					RawDataActItr->Id, RawDataActItr->Class, CurrLoc, CurrQuat);

				// If actor is skeletal, save bones data as well
				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(RawDataActItr->Entity))
				{
					// Bson array of bones
					bson_t BsonBonesArr;
					bson_init(&BsonBonesArr);

					// automatic key generation 
					const char* key;
					char keybuf[16];
					size_t keylen;
					int keynum = 0;

					// Get skeletal mesh component
					USkeletalMeshComponent* SkelComp = SkelAct->GetSkeletalMeshComponent();

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					BSON_APPEND_ARRAY_BEGIN(&BsonActorEntry, "bones", &BsonBonesArr);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						bson_t BsonBoneEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						keylen = bson_uint32_to_string(keynum, &key, keybuf, sizeof keybuf);
						bson_append_document(&BsonBonesArr, key, keylen, &BsonBoneEntry);
						keynum++;

					}
					bson_append_array_end(&BsonActorEntry, &BsonBonesArr);

				}
				// Add entity to Bson array
				bson_t BsonActorEntrydocument;
				bson_init(&BsonActorEntrydocument);
				BSON_APPEND_DOCUMENT(&BsonActorEntrydocument, "document", &BsonActorEntry);
				bson_concat(&OutBsonEntitiesArr, &BsonActorEntrydocument);
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}
}

// Add components
void FSLRawDataWriterMongo::AddComponents(bson_t& OutBsonEntitiesArr)
{
	// Iterate components
	for (auto RawDataCompItr(WorkerParent->RawDataComponents.CreateIterator()); RawDataCompItr; ++RawDataCompItr)
	{
		if (RawDataCompItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataCompItr->Entity->GetComponentLocation();
			const FQuat CurrQuat = RawDataCompItr->Entity->GetComponentQuat();
			if (FVector::DistSquared(CurrLoc, RawDataCompItr->PrevLoc) > WorkerParent->DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataCompItr->PrevLoc = CurrLoc;

				// Get current entry as Bson object
				bson_t BsonCompEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
					RawDataCompItr->Id, RawDataCompItr->Class, CurrLoc, CurrQuat);

				// If comp is skeletal, save bones data as well
				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(RawDataCompItr->Entity))
				{
					// Bson array of bones
					bson_t BsonBonesArr;
					bson_init(&BsonBonesArr);

					// automatic key generation 
					const char* key;
					char keybuf[16];
					size_t keylen;
					int keynum = 0;

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					BSON_APPEND_ARRAY_BEGIN(&BsonCompEntry, "bones", &BsonBonesArr);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						bson_t BsonBoneEntry = FSLRawDataWriterMongo::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						//generate key for bson document automatically
						keylen = bson_uint32_to_string(keynum, &key, keybuf, sizeof keybuf);
						bson_append_document(&BsonBonesArr, key, keylen, &BsonBoneEntry);
						keynum++;
					}
					bson_append_array_end(&BsonCompEntry, &BsonBonesArr);

				}
				// Add entity to Bson array
				bson_t BsonCompEntrydocument;
				bson_init(&BsonCompEntrydocument);
				BSON_APPEND_DOCUMENT(&BsonCompEntrydocument, "document", &BsonCompEntry);
				bson_concat(&OutBsonEntitiesArr, &BsonCompEntrydocument);
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
}

// Get entry as bson object
bson_t FSLRawDataWriterMongo::GetAsBsonEntry(const FString& InId,
	const FString& InClass,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// New Bson entity object
	bson_t BsonObj;
	bson_init(&BsonObj);

	// Add "id" field if available (bones have no separate ids)
	if (!InId.IsEmpty())
	{
		BSON_APPEND_UTF8(&BsonObj, "id", TCHAR_TO_UTF8(*InId));
	}

	// Add "class" field
	BSON_APPEND_UTF8(&BsonObj, "class", TCHAR_TO_UTF8(*InClass));

	// Create and add "loc" field
	bson_t loc;
	bson_init(&loc);
	BSON_APPEND_DOUBLE(&loc, "x", ROSLoc.X);
	BSON_APPEND_DOUBLE(&loc, "y", ROSLoc.Y);
	BSON_APPEND_DOUBLE(&loc, "z", ROSLoc.Z);
	BSON_APPEND_DOCUMENT(&BsonObj, "loc", &loc);

	// Create and add "rot" field
	bson_t rot;
	bson_init(&rot);
	BSON_APPEND_DOUBLE(&rot, "x", ROSQuat.X);
	BSON_APPEND_DOUBLE(&rot, "y", ROSQuat.Y);
	BSON_APPEND_DOUBLE(&rot, "z", ROSQuat.Z);
	BSON_APPEND_DOCUMENT(&BsonObj, "rot", &rot);

	return BsonObj;
}

// Write entry to db
void FSLRawDataWriterMongo::WriteToMongo(bson_t*& InRootObj, mongoc_collection_t* &collection)
{
	bson_error_t error;
	//insert the BsonObject in to mongo database 
	if (!mongoc_collection_insert_one(collection, InRootObj, NULL, NULL, &error)) {
		fprintf(stderr, "%s\n", error.message);
	}
	else {
		UE_LOG(LogTemp,Warning, TEXT("Data has been stored in mongo."));
	}
}
#endif //WITH_LIBMONGO
