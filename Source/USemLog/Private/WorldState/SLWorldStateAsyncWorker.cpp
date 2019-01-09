// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateAsyncWorker.h"
#include "SLMappings.h"
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

// Init writer, load items from sl mapping singleton
bool FSLWorldStateAsyncWorker::Create(UWorld* InWorld,
	ESLWorldStateWriterType WriterType,
	float DistanceStepSize,
	float RotationStepSize,
	const FString& EpisodeId,
	const FString& Location,
	const FString& HostIp,
	const uint16 HostPort)
{
	// Cache the world pointer
	World = InWorld;

	// Set the square of the distance threshold for objects to be logged
	DistanceStepSizeSquared = DistanceStepSize * DistanceStepSize;

	// Create the writer object
	switch(WriterType) 
	{
	case ESLWorldStateWriterType::Json:
		Writer = MakeShareable(new FSLWorldStateWriterJson(
			DistanceStepSize, RotationStepSize, Location, EpisodeId));
		break;
	case ESLWorldStateWriterType::Bson:
		Writer = MakeShareable(new FSLWorldStateWriterBson(
			DistanceStepSize, RotationStepSize, Location, EpisodeId));
		break;
	case ESLWorldStateWriterType::Mongo:
		Writer = MakeShareable(new FSLWorldStateWriterMongo(
			DistanceStepSize, RotationStepSize, Location, EpisodeId, HostIp, HostPort));
		break;
	default:
		Writer = MakeShareable(new FSLWorldStateWriterJson(
			DistanceStepSize, RotationStepSize, Location, EpisodeId));
		break;
	}

	// Writer could not be created
	if (!Writer.IsValid() || !Writer->IsReady())
	{
		return false;
	}

	// Make sure the semantic items are initialized
	FSLMappings::GetInstance()->Init(World);

	// Iterate through the semantically annotated objects
	for (const auto& ItemPair : FSLMappings::GetInstance()->GetItemMap())
	{
		// Take into account only objects with transform data (AActor, USceneComponents)
		if (AActor* ObjAsActor = Cast<AActor>(ItemPair.Value.Obj))
		{
			if (ASLSkeletalMeshActor* ObjAsSLSkelAct = Cast<ASLSkeletalMeshActor>(ObjAsActor))
			{
				SkeletalActorPool.Emplace(
					TSLItemState<ASLSkeletalMeshActor>(ItemPair.Value, ObjAsSLSkelAct));
			}
			else
			{
				NonSkeletalActorPool.Emplace(
					TSLItemState<AActor>(ItemPair.Value, ObjAsActor));
			}
		}
		else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(ItemPair.Value.Obj))
		{
			if (USkeletalMeshComponent* ObjAsSkelMeshComponent = Cast<USkeletalMeshComponent>(ObjAsSceneComp))
			{
				/*SkeletalComponentPool.Emplace(
					TSLItemState<USkeletalMeshComponent>(ItemPair.Value, ObjAsSkelMeshComponent));*/
			}
			else
			{
				NonSkeletalComponentPool.Emplace(
					TSLItemState<USceneComponent>(ItemPair.Value, ObjAsSceneComp));
			}
		}
	}

	// Can start working
	return true;
}

// Remove all items that are semantically marked as static
void FSLWorldStateAsyncWorker::RemoveStaticItems()
{
	// Non-skeletal actors
	for (auto Itr(NonSkeletalActorPool.CreateIterator()); Itr; ++Itr)
	{
		if (FTags::HasKeyValuePair(Itr->Entity.Get(), "SemLog", "Mobility", "Static"))
		{
			Itr.RemoveCurrent();
		}
	}
	NonSkeletalActorPool.Shrink();

	// Skeletal actors
	for (auto Itr(SkeletalActorPool.CreateIterator()); Itr; ++Itr)
	{
		if (FTags::HasKeyValuePair(Itr->Entity.Get(), "SemLog", "Mobility", "Static"))
		{
			Itr.RemoveCurrent();
		}
	}
	SkeletalActorPool.Shrink();

	// Non-skeletal scene components
	for (auto Itr(NonSkeletalComponentPool.CreateIterator()); Itr; ++Itr)
	{
		if (FTags::HasKeyValuePair(Itr->Entity.Get(), "SemLog", "Mobility", "Static"))
		{
			Itr.RemoveCurrent();
		}
	}
	NonSkeletalComponentPool.Shrink();

	//// Skeletal components
	//for (auto Itr(SkeletalComponentPool.CreateIterator()); Itr; ++Itr)
	//{
	//	if (!FTags::HasKeyValuePair(Itr->Entity.Get(), "SemLog", "Mobility", "Dynamic"))
	//	{
	//		Itr.RemoveCurrent();
	//	}
	//}
	//SkeletalComponentPool.Shrink();
}

// Async work done here
void FSLWorldStateAsyncWorker::DoWork()
{
	Writer->Write(NonSkeletalActorPool, SkeletalActorPool, NonSkeletalComponentPool,
		World->GetTimeSeconds());
}

// Needed by the engine API
FORCEINLINE TStatId FSLWorldStateAsyncWorker::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSLWorldStateAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
}
