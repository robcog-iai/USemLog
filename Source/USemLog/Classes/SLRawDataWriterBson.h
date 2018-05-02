// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLRawDataWriter.h"

// Forward declaration
class FSLRawDataAsyncWorker;

/**
 * 
 */
class FSLRawDataWriterBson : public FSLRawDataWriter
{
public:
	// Default constr
	FSLRawDataWriterBson();

	// Constructor with init
	FSLRawDataWriterBson(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId);

	// Destr
	virtual ~FSLRawDataWriterBson();

	// Init
	void Init(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId);

	// Called to write the data
	virtual void WriteData() override;

private:
	// Set the file handle for the logger
	void SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId);

	// Add actors
	void AddActors(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr);

	// Add components
	void AddComponents(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr);

	// Get entry as Bson object
	TSharedPtr<FJsonObject> GetAsBsonEntry(const FString& InId,
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
