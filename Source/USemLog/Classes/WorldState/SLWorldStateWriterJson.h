// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLWorldStateWriter.h"

// Forward declaration
class FSLWorldStateAsyncWorker;

/**
 * Raw data logger to json format
 */
class FSLWorldStateWriterJson : public ISLWorldStateWriter
{
public:
	// Constructor
	FSLWorldStateWriterJson();

	// Init constructor
	FSLWorldStateWriterJson(const FSLWorldStateWriterParams& InParams);

	// Destr
	virtual ~FSLWorldStateWriterJson();

	// Init
	virtual void Init(const FSLWorldStateWriterParams& InParams) override;

	// Write the data (it also removes invalid items from the array -- e.g. deleted ones)
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) override;

private:
	// Set the file handle for the logger
	bool SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);

	// Get non skeletal actors as json array
	void AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get skeletal actors as json array
	void AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get non skeletal components as json array
	void AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool, 
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get key value pairs as json entry
	TSharedPtr<FJsonObject> GetAsJsonEntry(const TMap<FString, FString>& InKeyValMap,
		const FVector& InLoc, const FQuat& InQuat);

	// Write entry to file
	void WriteToFile(const TSharedPtr<FJsonObject>& InRootObj);

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
