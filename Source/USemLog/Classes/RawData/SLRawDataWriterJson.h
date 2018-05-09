// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLRawDataWriter.h"

// Forward declaration
class FSLRawDataAsyncWorker;

/**
 * Raw data logger to json format
 */
class FSLRawDataWriterJson : public FSLRawDataWriter
{
public:
	// Default constr
	FSLRawDataWriterJson();

	// Constructor with init
	FSLRawDataWriterJson(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId);

	// Destr
	virtual ~FSLRawDataWriterJson();

	// Init
	void Init(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId);

	// Called to write the data
	virtual void WriteData() override;

private:
	// Set the file handle for the logger
	void SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);

	// Add actors
	void AddActors(TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Add components
	void AddComponents(TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr);

	// Get entry as json object
	TSharedPtr<FJsonObject> GetAsJsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Write entry to file
	void WriteToFile(const TSharedPtr<FJsonObject>& InRootObj);

	// Pointer to worker parent (access to raw data structure)
	FSLRawDataAsyncWorker* WorkerParent;

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
