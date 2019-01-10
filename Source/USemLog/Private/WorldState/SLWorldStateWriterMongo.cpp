// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterMongo.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"
#if WITH_LIBMONGO
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <string.h>
#endif //WITH_LIBMONGO

// Constr
FSLWorldStateWriterMongo::FSLWorldStateWriterMongo(float DistanceStepSize, float RotationStepSize, 
	const FString& Location, const FString& EpisodeId,
	const FString& HostIP, uint16 HostPort) :
	ISLWorldStateWriter(DistanceStepSize, RotationStepSize)
{
	bIsReady = FSLWorldStateWriterMongo::Connect(Location, EpisodeId, HostIP, HostPort);
}

// Destr
FSLWorldStateWriterMongo::~FSLWorldStateWriterMongo()
{
}

// Called to write the data
void FSLWorldStateWriterMongo::Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	float Timestamp)
{
#if WITH_LIBMONGO
	try
	{
		// Create a bson document and array to store the entities
		bsoncxx::builder::basic::document bson_doc{};
		bsoncxx::builder::basic::array bson_arr{};

		// Add entities to the bson array
		FSLWorldStateWriterMongo::AddNonSkeletalActors(NonSkeletalActorPool, bson_arr);
		FSLWorldStateWriterMongo::AddSkeletalActors(SkeletalActorPool, bson_arr);
		FSLWorldStateWriterMongo::AddNonSkeletalComponents(NonSkeletalComponentPool, bson_arr);


		// Avoid inserting empty entries
		using bsoncxx::builder::basic::kvp;
		if (!bson_arr.view().empty())
		{
			bson_doc.append(kvp("timestamp", bsoncxx::types::b_double{Timestamp}));
			bson_doc.append(kvp("entities", bson_arr));
			mongo_coll.insert_one(bson_doc.view());
		}
	}
	catch (const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
	}
#endif //WITH_LIBMONGO
}

// Connect to the database
bool FSLWorldStateWriterMongo::Connect(const FString& DB, const FString& EpisodeId, const FString& IP, uint16 Port)
{
#if WITH_LIBMONGO
	try
	{
		// Get current mongo instance, or create a new one (static variable)
		mongocxx::instance::current();

		// Create the connection URI
		FString Uri = TEXT("mongodb://") + IP + TEXT(":") + FString::FromInt(Port);
		mongocxx::uri mongo_uri(TCHAR_TO_UTF8(*Uri));
		UE_LOG(LogTemp, Log, TEXT("%s::%d Client URI=%s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(mongo_uri.to_string().c_str()));

		// Create the mongo client/connection
		// drivers are designed to succeed during client construction,
		// regardless of the state of servers
		// i.e. can return true even if it is no connected to the server
		mongo_conn = mongocxx::client {mongo_uri};
		if (!mongo_conn)
		{
			// Something went wrong
			return false;
		}

		mongo_db = mongo_conn[TCHAR_TO_UTF8(*DB)];
		mongo_coll = mongo_db[TCHAR_TO_UTF8(*EpisodeId)];
	}
	catch(const std::exception& xcp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Exception: %s"),
			TEXT(__FUNCTION__), __LINE__, UTF8_TO_TCHAR(xcp.what()));
		return false;
	}
	return true;
#elif
	return false;
#endif //WITH_LIBMONGO
}

#if WITH_LIBMONGO
// Get non skeletal actors as bson array
void FSLWorldStateWriterMongo::AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
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

			const float Distance = FVector::DistSquared(CurrLoc, Itr->PrevLoc);

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > DistanceStepSizeSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as bson document
				bsoncxx::builder::basic::document bson_doc_entry = FSLWorldStateWriterMongo::GetAsBsonEntry(
					TMap<FString, FString>{ {"id", Itr->Item.Id}, { "class", Itr->Item.Class } },
					CurrLoc, CurrQuat);

				out_bson_arr.append(bson_doc_entry);
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLMappings::GetInstance()->RemoveItem(Itr->Entity.Get());
		}
	}
}

// Get skeletal actors as bson array
void FSLWorldStateWriterMongo::AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	bsoncxx::builder::basic::array& out_bson_arr)
{
	out_bson_arr.append("skeletal_actors");
}

// Get non skeletal components as bson array
void FSLWorldStateWriterMongo::AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	bsoncxx::builder::basic::array& out_bson_arr)
{
	out_bson_arr.append("non_skeletal_components");
}

// Get key value pairs as bson entry
bsoncxx::builder::basic::document FSLWorldStateWriterMongo::GetAsBsonEntry(const TMap<FString, FString>& InKeyValMap,
	const FVector& InLoc, const FQuat& InQuat)
{
	using bsoncxx::builder::basic::kvp;
	// New bson document
	bsoncxx::builder::basic::document bson_doc{};

	//// Add key values
	//for (const auto& Pair : InKeyValMap)
	//{
	//	bson_doc.append(kvp(TCHAR_TO_UTF8(*Pair.Key), TCHAR_TO_UTF8(*Pair.Value)));
	//}

	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// Create a loc document
	bsoncxx::builder::basic::document bson_loc_doc{};
	bson_loc_doc.append(kvp("x", bsoncxx::types::b_double{ROSLoc.X}));
	bson_loc_doc.append(kvp("y", bsoncxx::types::b_double{ROSLoc.Y}));
	bson_loc_doc.append(kvp("z", bsoncxx::types::b_double{ROSLoc.Z}));
	bson_doc.append(kvp("loc", bson_loc_doc));

	// Create a rot document
	bsoncxx::builder::basic::document bson_rot_doc{};
	bson_rot_doc.append(kvp("x", bsoncxx::types::b_double{ROSQuat.X}));
	bson_rot_doc.append(kvp("y", bsoncxx::types::b_double{ROSQuat.Y}));
	bson_rot_doc.append(kvp("z", bsoncxx::types::b_double{ROSQuat.Z}));
	bson_rot_doc.append(kvp("w", bsoncxx::types::b_double{ROSQuat.W}));
	bson_doc.append(kvp("rot", bson_rot_doc));

	return bson_doc;
}
#endif //WITH_LIBMONGO
