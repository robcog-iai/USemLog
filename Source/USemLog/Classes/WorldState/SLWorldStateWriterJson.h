// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
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
	FSLWorldStateWriterJson(float DistanceStepSize, float RotationStepSize,
		const FString& Location, const FString& EpisodeId);

	// Destr
	virtual ~FSLWorldStateWriterJson();

	// Write the data (it also removes invalid items from the array -- e.g. deleted ones)
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) override;

private:
	// Set the file handle for the logger
	void SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);

	// Add non skeletal actors to a json format
	void AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Add semantically annotated skeletal actors to a json format
	void AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Add non skeletal components
	void AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool, 
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get entry as json object
	TSharedPtr<FJsonObject> GetAsJsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Write entry to file
	void WriteToFile(const TSharedPtr<FJsonObject>& InRootObj);

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
