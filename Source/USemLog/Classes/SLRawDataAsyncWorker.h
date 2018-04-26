// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Async/AsyncWork.h"
#include "Tags.h"

/**
* Raw data structure
*/
struct FSLRawDataObj
{
	// Default constructor
	FSLRawDataObj() {};

	// Constructor with init
	FSLRawDataObj(UObject* InObj,
		const FString& InId,
		FVector InPrevLoc = FVector(BIG_NUMBER)) : 
		Obj(InObj),
		Id(InId),
		PrevLoc(InPrevLoc)
	{};

	// Object pointer
	UObject* Obj;

	// Unique id of the entity
	FString Id;

	// Previous location 
	FVector PrevLoc;
};

/**
* Async worker to log raw data
*/
class SLRawDataAsyncWorker : public FNonAbandonableTask
{
	// Needed in order to get access to private data from DoWork()
	friend class FAsyncTask<SLRawDataAsyncWorker>;
	//friend class FAutoDeleteAsyncTask<SLRawDataAsyncWorker>;

	// Pointer to the world
	UWorld* World;

	// Array of semantically annotated entities to be logged
	TArray<FSLRawDataObj> RawDataObjects;

	// Constructor
	SLRawDataAsyncWorker(UWorld* InWorld) : World(InWorld) {}

	// Destructor
	~SLRawDataAsyncWorker() {}

	// Async work done here
	void DoWork()
	{
		for (uint32 x = 1; x < 100; x++)
		{
			for (uint32 i = 1; i < 65000; i++)
			{
				FVector A(FMath::RandRange(0.f, (float)i));
				FVector B(FMath::RandRange(0.f, (float)i));
				float Distance = FVector::Distance(A, B);
			}
		}
		//for (auto& RawDataObj : RawDataObjects)
		//{
		//	UE_LOG(LogTemp, Error, TEXT("[%s][%d] Id=%s, Loc=%s"),
		//		TEXT(__FUNCTION__), __LINE__, *RawDataObj.Id, *RawDataObj.PrevLoc.ToString());
		//	if (AActor* ObjAsActor = Cast<AActor>(RawDataObj.Obj))
		//	{
		//		RawDataObj.PrevLoc = ObjAsActor->GetActorLocation();
		//	}
		//	else if (USceneComponent* ObjAsComp = Cast<USceneComponent>(RawDataObj.Obj))
		//	{
		//		RawDataObj.PrevLoc = ObjAsComp->GetComponentLocation();
		//	}
		//}
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] !! !! !! DOWORK Task done !! !! !! "), TEXT(__FUNCTION__), __LINE__);
	}

	// Needed by the engine API
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(SLRawDataAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
	}

public:
	// Set all semantically logged objects
	void SetAllSemanticallyLoggedObjects()
	{
		// Get all objects with the SemLog tag type 
		TMap<UObject*, TMap<FString, FString>> ObjsToKeyValuePairs =
			FTags::GetObjectsToKeyValuePairs(World, TEXT("SemLog"));

		// Add all objects to the raw data object structure
		for (const auto& ObjToKVP : ObjsToKeyValuePairs)
		{
			// Take into account only objects with an id and class value set
			if (ObjToKVP.Value.Contains("Id") && ObjToKVP.Value.Contains("Class"))
			{
				const FString Id = ObjToKVP.Value["Id"];
				RawDataObjects.Add(FSLRawDataObj(ObjToKVP.Key, Id));
			}
		}
	}

	// Remove all non-dynamic objects from array
	void RemoveAllNonDynamicObjects()
	{
		for (auto RawDataObjItr(RawDataObjects.CreateIterator()); RawDataObjItr; ++RawDataObjItr)
		{
			if (AActor* ObjAsActor = Cast<AActor>(RawDataObjItr->Obj))
			{
				if (!FTags::HasKeyValuePair(ObjAsActor, "SemLog", "LogType", "Dynamic"))
				{
					RawDataObjItr.RemoveCurrent();
				}
			}
			else if (USceneComponent* ObjAsComp = Cast<USceneComponent>(RawDataObjItr->Obj))
			{
				if (!FTags::HasKeyValuePair(ObjAsComp, "SemLog", "LogType", "Dynamic"))
				{
					RawDataObjItr.RemoveCurrent();
				}
			}
			else
			{
				// Object does not belong a base class that has a transformation, remove
				RawDataObjItr.RemoveCurrent();
			}
		}
		RawDataObjects.Shrink();
	}
};
