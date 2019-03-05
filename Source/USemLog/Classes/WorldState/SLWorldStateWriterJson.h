// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLWorldStateWriter.h"

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

	// Finish
	virtual void Finish() override;

	// Write the data (it also removes invalid items from the array -- e.g. deleted ones)
	/*virtual void Write(TArray<TSLEntityPreviousPose<AActor>>& NonSkeletalActorPool,
		TArray<TSLEntityPreviousPose<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLEntityPreviousPose<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) override;*/

	void Write2(float Timestamp,
		TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		bool bCheckAndRemoveInvalidEntities = true) override;

private:
	// Set the file handle for the logger
	bool SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);

	// Get non skeletal actors as json array
	/*void AddNonSkeletalActors(TArray<TSLEntityPreviousPose<AActor>>& NonSkeletalActorPool,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);*/
	void AddActorEntities(TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get non skeletal components as json array
	/*void AddNonSkeletalComponents(TArray<TSLEntityPreviousPose<USceneComponent>>& NonSkeletalComponentPool, 
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);*/
	void AddComponentEntities(TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get skeletal actors as json array
	/*void AddSkeletalActors(TArray<TSLEntityPreviousPose<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);*/
	void AddSkeletalEntities(TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get key value pairs as json entry
	TSharedPtr<FJsonObject> GetAsJsonEntry(const TMap<FString, FString>& InKeyValMap,
		const FVector& InLoc, const FQuat& InQuat);

	// Write entry to file
	void WriteToFile(const TSharedPtr<FJsonObject>& InRootObj);

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
