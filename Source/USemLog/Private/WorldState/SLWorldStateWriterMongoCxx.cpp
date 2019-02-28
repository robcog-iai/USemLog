// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterMongoCxx.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"
#if SL_WITH_LIBMONGO_CXX
THIRD_PARTY_INCLUDES_START
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <string.h>
#include <iostream>
THIRD_PARTY_INCLUDES_END
using bsoncxx::builder::basic::kvp;
#endif //SL_WITH_LIBMONGO_CXX

// Constr
FSLWorldStateWriterMongoCxx::FSLWorldStateWriterMongoCxx()
{
	bIsInit = false;
}

FSLWorldStateWriterMongoCxx::FSLWorldStateWriterMongoCxx(const FSLWorldStateWriterParams& InParams)
{
	bIsInit = false;
	FSLWorldStateWriterMongoCxx::Init(InParams);
}


// Destr
FSLWorldStateWriterMongoCxx::~FSLWorldStateWriterMongoCxx()
{
}

// Init
void FSLWorldStateWriterMongoCxx::Init(const FSLWorldStateWriterParams& InParams)
{
	MinLinearDistanceSquared = InParams.LinearDistanceSquared;
	MinAngularDistance = InParams.AngularDistance;
	bIsInit = FSLWorldStateWriterMongoCxx::Connect(InParams.Location, InParams.EpisodeId, InParams.ServerIp, InParams.ServerPort);
}

// Finish
void FSLWorldStateWriterMongoCxx::Finish()
{
	if (bIsInit)
	{
		FSLWorldStateWriterMongoCxx::CreateIndexes();
		bIsInit = false;
	}
}

// Called to write the data
void FSLWorldStateWriterMongoCxx::Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	float Timestamp)
{
#if SL_WITH_LIBMONGO_CXX
		// Create a bson document and array to store the entities
		bsoncxx::builder::basic::document bson_doc{};
		bsoncxx::builder::basic::array bson_arr{};

		// Add entities to the bson array
		FSLWorldStateWriterMongoCxx::AddNonSkeletalActors(NonSkeletalActorPool, bson_arr);
		FSLWorldStateWriterMongoCxx::AddSkeletalActors(SkeletalActorPool, bson_arr);
		FSLWorldStateWriterMongoCxx::AddNonSkeletalComponents(NonSkeletalComponentPool, bson_arr);

		// Avoid inserting empty entries
		if (!bson_arr.view().empty())
		{
			bson_doc.append(kvp("timestamp", bsoncxx::types::b_double{Timestamp}));
			bson_doc.append(kvp("entities", bson_arr));
			try
			{
				mongo_coll.insert_one(bson_doc.view());
			}
			catch (const std::exception& xcp)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
					TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
			}
		}
#endif //SL_WITH_LIBMONGO_CXX
}

// Connect to the database
bool FSLWorldStateWriterMongoCxx::Connect(const FString& DBName, const FString& EpisodeId, const FString& ServerIp, uint16 ServerPort)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Params: DBName=%s; Collection=%s; IP=%s; Port=%d;"),
		TEXT(__FUNCTION__), __LINE__, *DBName, *EpisodeId, *ServerIp, ServerPort);
#if SL_WITH_LIBMONGO_CXX
	try
	{
		// Get current mongo instance, or create a new one (static variable)
		mongocxx::instance::current();

		//// Client connection options
		mongocxx::options::client client_options;
		//mongocxx::options::ssl ssl_options;

		//// If the server certificate is not signed by a well-known CA,
		//// you can set a custom CA file with the `ca_file` option.
		//// ssl_options.ca_file("/path/to/custom/cert.pem");

		//// If you want to disable certificate verification, you
		//// can set the `allow_invalid_certificates` option.
		//ssl_options.allow_invalid_certificates(true);

		//client_options.ssl_opts(ssl_options);

		// Create the connection URI
		FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);

		// Create the mongo client/connection
		// drivers are designed to succeed during client construction,
		// regardless of the state of servers
		// i.e. can return true even if it is no connected to the server
		//mongo_conn = mongocxx::client{ mongocxx::uri {}};
		//mongo_conn = mongocxx::client{ mongocxx::uri {}, client_options };
		mongo_conn = mongocxx::client { mongocxx::uri {TCHAR_TO_UTF8(*Uri)}, client_options};
		//mongo_conn = mongocxx::client{ mongocxx::uri{"mongodb://127.0.0.1"}};
		if (mongo_conn)
		{
			//FString MongoUri = FString(mongo_conn.uri().to_string().c_str());
			//UE_LOG(LogTemp, Error, TEXT("%s::%d Mongo client with URI: %s "),
			//	TEXT(__FUNCTION__), __LINE__, *MongoUri);
		}
		else
		{
			// Something went wrong
			return false;
		}

		// Dummy call to make sure that the server is online
		mongo_conn.list_databases();

		// Set/create the mongo database and collection
		mongo_db = mongo_conn[TCHAR_TO_UTF8(*DBName)];
		mongo_coll = mongo_db[TCHAR_TO_UTF8(*EpisodeId)];
	}
	catch(const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		return false;
	}
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_CXX
}

// Create indexes from the logged data, usually called after logging
bool FSLWorldStateWriterMongoCxx::CreateIndexes()
{
#if SL_WITH_LIBMONGO_CXX
	if (!bIsInit)
	{
		return false;
	}

	// Create indexes on the database
	try
	{
		/*mongocxx::options::index index_options{};*/
		mongo_coll.create_index(bsoncxx::builder::basic::make_document(kvp("timestamp", 1))/*, index_options*/);
		return true;
	}
	catch (const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		return false;
	}
#else
	return false;
#endif //SL_WITH_LIBMONGO_CXX
}

#if SL_WITH_LIBMONGO_CXX
// Get non skeletal actors as bson array
void FSLWorldStateWriterMongoCxx::AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	bsoncxx::builder::basic::array& out_bson_arr)
{
	// Iterate items
	for (auto Itr(NonSkeletalActorPool.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Entity->GetActorLocation();
			const FQuat CurrQuat = Itr->Entity->GetActorQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > MinLinearDistanceSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Create new local document to store the entity data
				bsoncxx::builder::basic::document bson_entity_doc{};
					
				// Add the id and class names
				bson_entity_doc.append(kvp("id", TCHAR_TO_UTF8(*Itr->Item.Id)));
				bson_entity_doc.append(kvp("class", TCHAR_TO_UTF8(*Itr->Item.Class)));

				// Add the pose information
				FSLWorldStateWriterMongoCxx::AddPoseToDocument(CurrLoc, CurrQuat, bson_entity_doc);

				// Add document to array
				out_bson_arr.append(bson_entity_doc);
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLObjectsManager::GetInstance()->RemoveObject(Itr->Entity.Get());
		}
	}
}

// Get skeletal actors as bson array
void FSLWorldStateWriterMongoCxx::AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	bsoncxx::builder::basic::array& out_bson_arr)
{
	// Iterate items
	for (auto Itr(SkeletalActorPool.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Entity->GetActorLocation();
			const FQuat CurrQuat = Itr->Entity->GetActorQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > MinLinearDistanceSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Create new local document to store the entity data
				bsoncxx::builder::basic::document bson_entity_doc{};

				// Add the id and class names
				bson_entity_doc.append(kvp("id", TCHAR_TO_UTF8(*Itr->Item.Id)));
				bson_entity_doc.append(kvp("class", TCHAR_TO_UTF8(*Itr->Item.Class)));

				// Add the pose information
				FSLWorldStateWriterMongoCxx::AddPoseToDocument(CurrLoc, CurrQuat, bson_entity_doc);

				// Array of bones
				bsoncxx::builder::basic::array bson_bone_arr{};

				// Check is the skeletal actor component is valid and has a class mapping of the bone
				if (USLSkeletalDataAsset* SkelMapDataAsset = Itr->Entity->GetSkeletalMapDataAsset())
				{
					if (USkeletalMeshComponent* SkelComp = Itr->Entity->GetSkeletalMeshComponent())
					{
						// Iterate through the bones of the skeletal mesh
						for (const auto& Pair : SkelMapDataAsset->BoneClasses)
						{
							const FVector CurrLoc = SkelComp->GetBoneLocation(Pair.Key);
							const FQuat CurrQuat = SkelComp->GetBoneQuaternion(Pair.Key);

							bsoncxx::builder::basic::document bson_bone_doc{};

							// Add the id and class names
							bson_bone_doc.append(kvp("bone", TCHAR_TO_UTF8(*Pair.Key.ToString())));
							bson_bone_doc.append(kvp("class", TCHAR_TO_UTF8(*Pair.Value)));

							// Add the pose information
							FSLWorldStateWriterMongoCxx::AddPoseToDocument(CurrLoc, CurrQuat, bson_bone_doc);

							// Add bone to  array
							bson_bone_arr.append(bson_bone_doc);
						}
					}
				}
				// Add bones to doc
				bson_entity_doc.append(kvp("bones", bson_bone_arr));

				// Add document to array
				out_bson_arr.append(bson_entity_doc);
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLObjectsManager::GetInstance()->RemoveObject(Itr->Entity.Get());
		}
	}
}

// Get non skeletal components as bson array
void FSLWorldStateWriterMongoCxx::AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	bsoncxx::builder::basic::array& out_bson_arr)
{
	// Iterate items
	for (auto Itr(NonSkeletalComponentPool.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Entity->GetComponentLocation();
			const FQuat CurrQuat = Itr->Entity->GetComponentQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > MinLinearDistanceSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Create new local document to store the entity data
				bsoncxx::builder::basic::document bson_entity_doc{};

				// Add the id and class names
				bson_entity_doc.append(kvp("id", TCHAR_TO_UTF8(*Itr->Item.Id)));
				bson_entity_doc.append(kvp("class", TCHAR_TO_UTF8(*Itr->Item.Class)));

				// Add the pose information
				FSLWorldStateWriterMongoCxx::AddPoseToDocument(CurrLoc, CurrQuat, bson_entity_doc);

				// Add document to array
				out_bson_arr.append(bson_entity_doc);
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLObjectsManager::GetInstance()->RemoveObject(Itr->Entity.Get());
		}
	}
}

// Add pose data to document
void FSLWorldStateWriterMongoCxx::AddPoseToDocument(const FVector& InLoc, const FQuat& InQuat,
	bsoncxx::builder::basic::document& out_doc)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// Create a loc document
	bsoncxx::builder::basic::document bson_loc_doc{};
	bson_loc_doc.append(kvp("x", bsoncxx::types::b_double{ROSLoc.X}));
	bson_loc_doc.append(kvp("y", bsoncxx::types::b_double{ROSLoc.Y}));
	bson_loc_doc.append(kvp("z", bsoncxx::types::b_double{ROSLoc.Z}));
	out_doc.append(kvp("loc", bson_loc_doc));

	// Create a rot document
	bsoncxx::builder::basic::document bson_rot_doc{};
	bson_rot_doc.append(kvp("x", bsoncxx::types::b_double{ROSQuat.X}));
	bson_rot_doc.append(kvp("y", bsoncxx::types::b_double{ROSQuat.Y}));
	bson_rot_doc.append(kvp("z", bsoncxx::types::b_double{ROSQuat.Z}));
	bson_rot_doc.append(kvp("w", bsoncxx::types::b_double{ROSQuat.W}));
	out_doc.append(kvp("rot", bson_rot_doc));
}
#endif //SL_WITH_LIBMONGO_CXX
