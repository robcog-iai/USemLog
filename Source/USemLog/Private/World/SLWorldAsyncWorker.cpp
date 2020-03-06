// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "World/SLWorldAsyncWorker.h"
#include "SLEntitiesManager.h"
#include "World/SLWorldWriterJson.h"
#include "World/SLWorldWriterBson.h"
#include "World/SLWorldWriterMongoC.h"
#include "World/SLWorldWriterMongoCxx.h"
#include "Tags.h"
#include "Animation/SkeletalMeshActor.h"

// Constructor
FSLWorldAsyncWorker::FSLWorldAsyncWorker()
{
	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Destructor
FSLWorldAsyncWorker::~FSLWorldAsyncWorker()
{
	Finish(true);
}

// Init writer, load items from sl mapping singleton
void FSLWorldAsyncWorker::Init(UWorld* InWorld,
	ESLWorldWriterType InWriterType,
	const FSLWorldWriterParams& InParams)
{
	if(!bIsInit)
	{
		// Pointer to the world
		World = InWorld;
		if(!World)
		{
			return;
		}
		
		// Cache the writer type
		WriterType = InWriterType;

		// Create the writer object
		switch(WriterType)
		{
		case ESLWorldWriterType::Json:
			Writer = MakeShareable(new FSLWorldWriterJson(InParams));
			break;
		case ESLWorldWriterType::Bson:
			Writer = MakeShareable(new FSLWorldWriterBson(InParams));
			break;
		case ESLWorldWriterType::MongoC:
			Writer = MakeShareable(new FSLWorldWriterMongoC(InParams));
			break;
		case ESLWorldWriterType::MongoCxx:
			Writer = MakeShareable(new FSLWorldWriterMongoCxx(InParams));
			break;
		default:
			Writer = MakeShareable(new FSLWorldWriterJson(InParams));
			break;
		}

		// Writer could not be created
		if (!Writer.IsValid() || !Writer->IsInit())
		{
			return;
		}

		// Iterate all annotated entities, ignore skeletal ones
		TArray<FSLEntity> SemanticEntities;
		FSLEntitiesManager::GetInstance()->GetSemanticDataArray(SemanticEntities);
		for (const auto& SemEntity : SemanticEntities)
		{
			// Take into account only objects with transform data (AActor, USceneComponents)
			if (AActor* ObjAsActor = Cast<AActor>(SemEntity.Obj))
			{
				// Continue if it is not a skeletal mesh actor
				if (!Cast<ASkeletalMeshActor>(ObjAsActor))
				{
					ActorEntitites.Emplace(TSLEntityPreviousPose<AActor>(ObjAsActor, SemEntity));
				}
			}
			else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(SemEntity.Obj))
			{
				// Continue if it is not a skeletal mesh component 
				if (!Cast<USkeletalMeshComponent>(ObjAsSceneComp))
				{
					ComponentEntities.Emplace(TSLEntityPreviousPose<USceneComponent>(ObjAsSceneComp, SemEntity));
				}
			}
		}

		// Get the skeletal data info
		TArray<USLSkeletalDataComponent*> SemanticSkeletalData;
		FSLEntitiesManager::GetInstance()->GetSemanticSkeletalDataArray(SemanticSkeletalData);
		for (const auto& SemSkelData : SemanticSkeletalData)
		{
			SkeletalEntities.Emplace(TSLEntityPreviousPose<USLSkeletalDataComponent>(
				SemSkelData, SemSkelData->OwnerSemanticData));
		}

		// Init the gaze handler
		GazeDataHandler.Init(World);

		// Can start working
		bIsInit = true;
	}
}

// Prepare worker for starting to log
void FSLWorldAsyncWorker::Start()
{
	if(!bIsStarted && bIsInit)
	{
		// Start the gaze handler
		GazeDataHandler.Start();

		bIsStarted = true;
	}
}

// Finish up worker
void FSLWorldAsyncWorker::Finish(bool bForced)
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		if (!bForced)
		{
			// Check if mongo writer
			if (Writer.IsValid())
			{
				if (WriterType == ESLWorldWriterType::MongoCxx)
				{
					// We cannot cast dynamically if it is not an UObject
					TSharedPtr<FSLWorldWriterMongoCxx> AsMongoCxxWriter = StaticCastSharedPtr<FSLWorldWriterMongoCxx>(Writer);
					// Finish writer (create database indexes for example)
					AsMongoCxxWriter->Finish();
				}
				else if (WriterType == ESLWorldWriterType::MongoC)
				{
					// We cannot cast dynamically if it is not an UObject
					TSharedPtr<FSLWorldWriterMongoC> AsMongoCWriter = StaticCastSharedPtr<FSLWorldWriterMongoC>(Writer);
					// Finish writer (create database indexes for example)
					AsMongoCWriter->Finish();
				}
			}

			GazeDataHandler.Finish();
		}
		
		bIsInit = false;
		bIsStarted = false;
		bIsFinished = true;
	}
}

// Remove all items that are semantically marked as static
void FSLWorldAsyncWorker::RemoveStaticItems()
{
	// Non-skeletal actors
	for (auto Itr(ActorEntitites.CreateIterator()); Itr; ++Itr)
	{
		if (FTags::HasKeyValuePair(Itr->Obj.Get(), "SemLog", "Mobility", "Static"))
		{
			Itr.RemoveCurrent();
		}
	}
	ActorEntitites.Shrink();

	// Non-skeletal scene components
	for (auto Itr(ComponentEntities.CreateIterator()); Itr; ++Itr)
	{
		if (FTags::HasKeyValuePair(Itr->Obj.Get(), "SemLog", "Mobility", "Static"))
		{
			Itr.RemoveCurrent();
		}
	}
	ComponentEntities.Shrink();

	// Skeletal components are probably always movable, so we just skip that step
}

// Async work done here
void FSLWorldAsyncWorker::DoWork()
{
	FSLGazeData GazeData;
	GazeDataHandler.GetData(GazeData);
	Writer->Write(World->GetTimeSeconds(), ActorEntitites, ComponentEntities, SkeletalEntities, GazeData);
}

// Needed by the engine API
FORCEINLINE TStatId FSLWorldAsyncWorker::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSLWorldAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
}
