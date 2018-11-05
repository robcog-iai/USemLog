// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateAsyncWorker.h"
#include "SLMappings.h"
// TODO use interface
#include "WorldState/SLWorldStateWriterJson.h"
#include "WorldState/SLWorldStateWriterBson.h"
#include "WorldState/SLWorldStateWriterMongo.h"
#include "Tags.h"

// Constructor
FSLWorldStateAsyncWorker::FSLWorldStateAsyncWorker()
{
}

// Destructor
FSLWorldStateAsyncWorker::~FSLWorldStateAsyncWorker()
{
}

// Init worker, load models to log from world
void FSLWorldStateAsyncWorker::Init(UWorld* InWorld, const float DistanceThreshold)
{
	// Init the mappings 
	FSLMappings::GetInstance()->Init(InWorld);

	// Set pointer to world (access to current timestamp)
	World = InWorld;

	// Set the square of the distance threshold for objects to be logged
	DistanceSquaredThreshold = DistanceThreshold * DistanceThreshold;

	//// Get all objects with the SemLog tag type 
	//TMap<UObject*, TMap<FString, FString>> ObjsToKeyValuePairs =
	//	FTags::GetObjectKeyValuePairsMap(InWorld, TEXT("SemLog"));

	//// Add static and dynamic objects with transform data
	//for (const auto& ObjToKVP : ObjsToKeyValuePairs)
	//{
	//	// Take into account only objects with an id and class value set
	//	const FString* IdPtr = ObjToKVP.Value.Find("Id");
	//	const FString* ClassPtr = ObjToKVP.Value.Find("Class");
	//	if (IdPtr && ClassPtr)
	//	{
	//		// Take into account only objects with transform data)
	//		if (AActor* ObjAsActor = Cast<AActor>(ObjToKVP.Key))
	//		{
	//			//WorldStateActors.Add(FSLWorldStateActor(TWeakObjectPtr<AActor>(ObjAsActor), Id));
	//			WorldStateActors.Add(TSLWorldStateEntity<AActor>(ObjAsActor, *IdPtr, *ClassPtr));
	//		}
	//		else if (USceneComponent* ObjAsComp = Cast<USceneComponent>(ObjToKVP.Key))
	//		{
	//			WorldStateComponents.Add(TSLWorldStateEntity<USceneComponent>(ObjAsComp, *IdPtr, *ClassPtr));
	//		}
	//	}
	//}

	// Iterate through the semantically annotated objects
	for (const auto& ItemPair : FSLMappings::GetInstance()->GetItemMap())
	{
		// Take into account only objects with transform data)
		if (AActor* ObjAsActor = Cast<AActor>(ItemPair.Value.Obj))
		{
			//WorldStateActors.Add(FSLWorldStateActor(TWeakObjectPtr<AActor>(ObjAsActor), Id));
			WorldStateActors.Add(TSLWorldStateEntity<AActor>(ObjAsActor, ItemPair.Value.Id, ItemPair.Value.Class));
		}
		else if (USceneComponent* ObjAsComp = Cast<USceneComponent>(ItemPair.Value.Obj))
		{
			WorldStateComponents.Add(TSLWorldStateEntity<USceneComponent>(ObjAsComp, ItemPair.Value.Id, ItemPair.Value.Class));
		}
	}
}

// Log data to json file
void FSLWorldStateAsyncWorker::SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	Writer = MakeShareable(new FSLWorldStateWriterJson(this, InLogDirectory, InEpisodeId));
}

// Log data to bson file
void FSLWorldStateAsyncWorker::SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	Writer = MakeShareable(new FSLWorldStateWriterBson(this, InLogDirectory, InEpisodeId));
}

// Log data to mongodb
void FSLWorldStateAsyncWorker::SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort)
{
	Writer = MakeShareable(new FSLWorldStateWriterMongo(this, InLogDB, InEpisodeId, InMongoIP, MongoPort));
}

// Remove all non-dynamic objects from arrays
void FSLWorldStateAsyncWorker::RemoveAllNonDynamicObjects()
{
	// Remove static/invalid actors
	for (auto WorldStateActItr(WorldStateActors.CreateIterator()); WorldStateActItr; ++WorldStateActItr)
	{
		if (WorldStateActItr->Entity.IsValid())
		{
			if (!FTags::HasKeyValuePair(WorldStateActItr->Entity.Get(), "SemLog", "Mobility", "Dynamic"))
			{
				WorldStateActItr.RemoveCurrent();
			}
		}
		else
		{
			WorldStateActItr.RemoveCurrent();
		}
	}
	WorldStateActors.Shrink();

	// Remove static/invalid components
	for (auto WorldStateCompItr(WorldStateComponents.CreateIterator()); WorldStateCompItr; ++WorldStateCompItr)
	{
		if (WorldStateCompItr->Entity.IsValid())
		{
			if (!FTags::HasKeyValuePair(WorldStateCompItr->Entity.Get(), "SemLog", "Mobility", "Dynamic"))
			{
				WorldStateCompItr.RemoveCurrent();
			}
		}
		else
		{
			WorldStateCompItr.RemoveCurrent();
		}
	}
	WorldStateComponents.Shrink();
}

// Async work done here
void FSLWorldStateAsyncWorker::DoWork()
{
	if (Writer.IsValid())
	{
		Writer->WriteData();
	}
}

// Needed by the engine API
FORCEINLINE TStatId FSLWorldStateAsyncWorker::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSLWorldStateAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
}
