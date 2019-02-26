// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Async/AsyncWork.h"
#include "ISLWorldStateWriter.h"
#include "SLStructs.h"
#include "SLSkeletalMeshActor.h"

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
	bool Create(UWorld* InWorld,
		ESLWorldStateWriterType WriterType,
		float LinearDistance,
		float AngularDistance,
		const FString& Location,
		const FString& EpisodeId,
		const FString& ServerIp = "",
		const uint16 ServerPort = 0);

	// Init worker, load models to log from world
	bool Create(UWorld* InWorld,
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
	TArray<TSLItemState<AActor>> NonSkeletalActorPool;

	// Array of semantically annotated skeletal actors
	TArray<TSLItemState<ASLSkeletalMeshActor>> SkeletalActorPool;

	// Array of semantically annotated components that are not skeletal
	TArray<TSLItemState<USceneComponent>> NonSkeletalComponentPool;

	// TODO USLSkeletalMeshComponent,
	// could not find out how to dynamically change type to point to a SLSkeletalDataAsset
	// Array of semantically annotated skeletal actors
	//TArray<TSLItemState<USkeletalMeshComponent>> SkeletalComponentPool;
};
