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
	FSLWorldStateAsyncWorker::Finish(true);
	UE_LOG(LogTemp, Error, TEXT("%s::%d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "), TEXT(__FUNCTION__), __LINE__);
}

// Init writer, load items from sl mapping singleton
bool FSLWorldStateAsyncWorker::Create(UObject* InParent,
	ESLWorldStateWriterType WriterType,
	float LinearDistance,
	float AngularDistance,
	const FString& Location,
	const FString& EpisodeId,
	const FString& ServerIp,
	const uint16 ServerPort)
{
	return FSLWorldStateAsyncWorker::Create(InParent, WriterType,
		FSLWorldStateWriterParams(LinearDistance, AngularDistance, Location, EpisodeId, ServerIp, ServerPort));
}

// Init writer, load items from sl mapping singleton
bool FSLWorldStateAsyncWorker::Create(UObject* InParent,
	ESLWorldStateWriterType WriterType,
	const FSLWorldStateWriterParams& InParams)
{
	if (!InParent || !InParent->GetWorld())
	{
		return false;
	}

	// Cache the world and parent pointer
	Parent = InParent;
	World = Parent->GetWorld();

	// Create the writer object
	switch(WriterType) 
	{
	case ESLWorldStateWriterType::Json:
		Writer = NewObject<USLWorldStateWriterJson>(Parent);
		Writer->Init(InParams);
		break;
	case ESLWorldStateWriterType::Bson:
		Writer = NewObject<USLWorldStateWriterBson>(Parent);
		Writer->Init(InParams);
		break;
	case ESLWorldStateWriterType::Mongo:
		Writer = NewObject<USLWorldStateWriterMongo>(Parent);
		//Writer->Init(InParams);
		break;
	default:
		Writer = NewObject<USLWorldStateWriterJson>(Parent);
		Writer->Init(InParams);
		break;
	}

	// Avoid GC
	if (UObject* WriterAsObj = Cast<UObject>(Writer))
	{
		WriterAsObj->SetInternalFlags(EInternalObjectFlags::Async);
		WriterAsObj->AddToRoot();
	}
	else
	{
		return false;
	}


	// Writer could not be created
	if (!Writer->IsInit())
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
		if (USLWorldStateWriterMongo* MongoWriter = Cast<USLWorldStateWriterMongo>(Writer))
		{
			// Create indexes
			MongoWriter->CreateIndexes();
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
