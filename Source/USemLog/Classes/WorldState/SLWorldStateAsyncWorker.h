// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Async/AsyncWork.h"
#include "ISLWorldStateWriter.h"
#include "SLStructs.h"
//#include "SLSkeletalMeshActor.h"

/**
* Type of world state loggers
*/
UENUM()
enum class ESLWorldStateWriterType : uint8
{
	Json					UMETA(DisplayName = "Json"),
	Bson					UMETA(DisplayName = "Bson"),
	MongoC					UMETA(DisplayName = "MongoC"),
	MongoCxx				UMETA(DisplayName = "MongoCxx")
};

/**
 * Async worker to log raw data
 */
class FSLWorldStateAsyncWorker : public FNonAbandonableTask
{
	// Needed if DoWork() needs access private data from this class
	friend class FAsyncTask<FSLWorldStateAsyncWorker>;
	//friend class FAutoDeleteAsyncTask<SLWorldStateAsyncWorker>;

public:
	// Constructor
	FSLWorldStateAsyncWorker();

	// Destructor
	virtual ~FSLWorldStateAsyncWorker();

	// Init worker, load models to log from world
	bool Init(UWorld* InWorld,
		ESLWorldStateWriterType InWriterType,
		const FSLWorldStateWriterParams& InParams);

	// Remove all non-movable semantic items from the update pool
	void RemoveStaticItems();

	// Finish up worker
	void Finish(bool bForced = false);

private:
	// FAsyncTask - async work done here
	void DoWork();

	// Needed by unreal internally
	FORCEINLINE TStatId GetStatId() const;

private:
	// Pointer to world (access to timestamps)
	UWorld* World;

	// Cache of the writer thy that is active
	ESLWorldStateWriterType WriterType;

	// Distance squared threshold
	float LinearDistanceSquared;

	// Raw data writer
	TSharedPtr<ISLWorldStateWriter> Writer;

	// Array of semantically annotated actors that are not skeletal
	TArray<TSLEntityPreviousPose<AActor>> ActorEntitites;
	
	// Array of semantically annotated components that are not skeletal
	TArray<TSLEntityPreviousPose<USceneComponent>> ComponentEntities;

	// Array of semantical skeletal data components
	TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>> SkeletalEntities;

	//// Array of semantically annotated skeletal actors
	//TArray<TSLEntityPreviousPose<ASLSkeletalMeshActor>> SkeletalActorPool; // TODO rm

	// TODO USLSkeletalMeshComponent,
	// could not find out how to dynamically change type to point to a SLSkeletalDataAsset
	// Array of semantically annotated skeletal actors
	//TArray<TSLItemState<USkeletalMeshComponent>> SkeletalComponentPool;
};
