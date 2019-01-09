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
		// Bson document
		auto bson_doc = bsoncxx::builder::basic::document{};

		// Bson array
		auto bson_arr = bsoncxx::builder::basic::array{};

		// Add entities to the bson array
		FSLWorldStateWriterMongo::AddNonSkeletalActors(NonSkeletalActorPool, bson_arr);
		FSLWorldStateWriterMongo::AddSkeletalActors(SkeletalActorPool, bson_arr);
		FSLWorldStateWriterMongo::AddNonSkeletalComponents(NonSkeletalComponentPool, bson_arr);

		// Avoid inserting empty entries
		if (!bson_arr.view().empty())
		{
			bson_doc.append(bsoncxx::builder::basic::kvp("timestamp", bsoncxx::types::b_double{Timestamp}));
			bson_doc.append(bsoncxx::builder::basic::kvp("entities", bson_arr));
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

		// Create the mongo client/connection
		// drivers are designed to succeed during client construction,
		// regardless of the state of servers
		// i.e. can return true even if it is no connected to the server
		mongo_conn = mongocxx::client {mongo_uri};
		if (!mongo_conn)
		{
			return false;
		}

		mongo_db = mongo_conn[TCHAR_TO_UTF8(*DB)];
		mongo_coll = mongo_db[TCHAR_TO_UTF8(*EpisodeId)];

		bsoncxx::document::value document = 
			bsoncxx::builder::basic::make_document(
				bsoncxx::builder::basic::kvp(
					"hello", "world"));

		mongo_coll.insert_one({});
		mongo_coll.insert_one(document.view());
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
	out_bson_arr.append("non_skeletal_actors");
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

//// Get key value pairs as bson entry
//bsoncxx::builder::basic::sub_document FSLWorldStateWriterMongo::GetAsBsonEntry(const TMap<FString, FString>& InKeyValMap,
//	const FVector& InLoc, const FQuat& InQuat)
//{
//	return bsoncxx::builder::basic::sub_document sub_doc;
//}
#endif //WITH_LIBMONGO
