// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "RawData/SLRawDataAsyncWorker.h"
#include "Tags.h"

// Constructor
FSLRawDataAsyncWorker::FSLRawDataAsyncWorker()
{
}

// Destructor
FSLRawDataAsyncWorker::~FSLRawDataAsyncWorker()
{
}

// Init worker, load models to log from world
void FSLRawDataAsyncWorker::Init(UWorld* InWorld, const float DistanceThreshold)
{
	// Create default writer
	Writer = MakeShareable(new FSLRawDataWriter());

	// Set pointer to world (access to current timestamp)
	World = InWorld;

	// Set the square of the distance threshold for objects to be logged
	DistanceSquaredThreshold = DistanceThreshold * DistanceThreshold;

	// Get all objects with the SemLog tag type 
	TMap<UObject*, TMap<FString, FString>> ObjsToKeyValuePairs =
		FTags::GetObjectsToKeyValuePairs(InWorld, TEXT("SemLog"));

	// Add static and dynamic objects with transform data
	for (const auto& ObjToKVP : ObjsToKeyValuePairs)
	{
		// Take into account only objects with an id and class value set
		if (ObjToKVP.Value.Contains("Id") && ObjToKVP.Value.Contains("Class"))
		{
			const FString Id = ObjToKVP.Value["Id"];
			const FString Class = ObjToKVP.Value["Class"];
			// Take into account only objects with transform data)
			if (AActor* ObjAsActor = Cast<AActor>(ObjToKVP.Key))
			{
				//RawDataActors.Add(FSLRawDataActor(TWeakObjectPtr<AActor>(ObjAsActor), Id));
				RawDataActors.Add(TSLRawDataEntity<AActor>(ObjAsActor, Id, Class));
			}
			else if (USceneComponent* ObjAsComp = Cast<USceneComponent>(ObjToKVP.Key))
			{
				RawDataComponents.Add(TSLRawDataEntity<USceneComponent>(ObjAsComp, Id, Class));
			}
		}
	}
}

// Log data to json file
void FSLRawDataAsyncWorker::SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	Writer = MakeShareable(new FSLRawDataWriterJson(this, InLogDirectory, InEpisodeId));
}

// Log data to bson file
void FSLRawDataAsyncWorker::SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	Writer = MakeShareable(new FSLRawDataWriterBson(this, InLogDirectory, InEpisodeId));
}

// Log data to mongodb
void FSLRawDataAsyncWorker::SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort)
{
	Writer = MakeShareable(new FSLRawDataWriterMongo(this, InLogDB, InEpisodeId, InMongoIP, MongoPort));
}

// Remove all non-dynamic objects from arrays
void FSLRawDataAsyncWorker::RemoveAllNonDynamicObjects()
{
	// Remove static/invalid actors
	for (auto RawDataActItr(RawDataActors.CreateIterator()); RawDataActItr; ++RawDataActItr)
	{
		if (RawDataActItr->Entity.IsValid())
		{
			if (!FTags::HasKeyValuePair(RawDataActItr->Entity.Get(), "SemLog", "LogType", "Dynamic"))
			{
				RawDataActItr.RemoveCurrent();
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}
	RawDataActors.Shrink();

	// Remove static/invalid components
	for (auto RawDataCompItr(RawDataComponents.CreateIterator()); RawDataCompItr; ++RawDataCompItr)
	{
		if (RawDataCompItr->Entity.IsValid())
		{
			if (!FTags::HasKeyValuePair(RawDataCompItr->Entity.Get(), "SemLog", "LogType", "Dynamic"))
			{
				RawDataCompItr.RemoveCurrent();
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
	RawDataComponents.Shrink();
}

// Async work done here
void FSLRawDataAsyncWorker::DoWork()
{
	Writer->WriteData();
}

// Needed by the engine API
FORCEINLINE TStatId FSLRawDataAsyncWorker::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSLRawDataAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
}
