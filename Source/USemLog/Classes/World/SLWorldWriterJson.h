// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLWorldWriter.h"

/**
 * Raw data logger to json format
 */
class FSLWorldWriterJson : public ISLWorldWriter
{
public:
	// Constructor
	FSLWorldWriterJson();

	// Init constructor
	FSLWorldWriterJson(const FSLWorldWriterParams& InParams);

	// Destr
	virtual ~FSLWorldWriterJson();

	// Init
	virtual void Init(const FSLWorldWriterParams& InParams) override;

	// Finish
	virtual void Finish() override;

	// Write the data (it also removes invalid items from the array -- e.g. deleted ones)
	void Write(float Timestamp,
		TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		FSLGazeData& GazeData,
		bool bCheckAndRemoveInvalidEntities = true) override;

private:
	// Set the file handle for the logger
	bool SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);
	
#if SL_WITH_JSON
	// Add non skeletal actors to json array
	void AddActorEntities(TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Add non skeletal components to json array
	void AddComponentEntities(TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Add skeletal actors to json array
	void AddSkeletalEntities(TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get key value pairs as json entry
	TSharedPtr<FJsonObject> GetAsJsonEntry(const TMap<FString, FString>& InKeyValMap,
		const FVector& InLoc, const FQuat& InQuat);

	// Write entry to file
	void WriteToFile(const TSharedPtr<FJsonObject>& InRootObj);
#endif SL_WITH_JSON
	
	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
