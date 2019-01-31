// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateAsyncWorker.h"
#include "SLMappings.h"
#include "WorldState/SLWorldStateWriterJson.h"
#include "WorldState/SLWorldStateWriterBson.h"
#include "WorldState/SLWorldStateWriterMongoC.h"
#include "WorldState/SLWorldStateWriterMongoCxx.h"
#include "Tags.h"

// Constructor
FSLWorldStateAsyncWorker::FSLWorldStateAsyncWorker()
{
}

// Destructor
FSLWorldStateAsyncWorker::~FSLWorldStateAsyncWorker()
{
	FSLWorldStateAsyncWorker::Finish(true);
}

// Init writer, load items from sl mapping singleton
bool FSLWorldStateAsyncWorker::Create(UWorld* InWorld,
	ESLWorldStateWriterType WriterType,
	float LinearDistance,
	float AngularDistance,
	const FString& Location,
	const FString& EpisodeId,
	const FString& ServerIp,
	const uint16 ServerPort)
{
	return FSLWorldStateAsyncWorker::Create(InWorld, WriterType,
		FSLWorldStateWriterParams(LinearDistance, AngularDistance, Location, EpisodeId, ServerIp, ServerPort));
}

// Init writer, load items from sl mapping singleton
bool FSLWorldStateAsyncWorker::Create(UWorld* InWorld,
	ESLWorldStateWriterType InWriterType,
	const FSLWorldStateWriterParams& InParams)
{
	// Pointer to the world
	World = InWorld;
	// Cache the writer type
	WriterType = InWriterType;

	// Create the writer object
	switch(WriterType)
	{
	case ESLWorldStateWriterType::Json:
		Writer = MakeShareable(new FSLWorldStateWriterJson(InParams));
		break;
	case ESLWorldStateWriterType::Bson:
		Writer = MakeShareable(new FSLWorldStateWriterBson(InParams));
		break;
	case ESLWorldStateWriterType::MongoC:
		Writer = MakeShareable(new FSLWorldStateWriterMongoC(InParams));
		break;
	case ESLWorldStateWriterType::MongoCxx:
		Writer = MakeShareable(new FSLWorldStateWriterMongoCxx(InParams));
		break;
	default:
		Writer = MakeShareable(new FSLWorldStateWriterJson(InParams));
		break;
	}

	// Writer could not be created
	if (!Writer.IsValid() || !Writer->IsInit())
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

// Finish up worker
void FSLWorldStateAsyncWorker::Finish(bool bForced)
{
	if (!bForced)
	{
		// Check if mongo writer
		if (Writer.IsValid() && WriterType == ESLWorldStateWriterType::MongoCxx)
		{
			// We cannot cast dynamically if it is not an UObject
			TSharedPtr<FSLWorldStateWriterMongoCxx> MongoWriter = StaticCastSharedPtr<FSLWorldStateWriterMongoCxx>(Writer);
			// Finish writer (create database indexes for example)
			MongoWriter->Finish();
		}
	}
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
