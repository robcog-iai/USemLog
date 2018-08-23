// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ISLRawDataWriter.h"
#if USE_LIBMONGO
#include "bson.h"
#endif //USE_LIBMONGO

// Forward declaration
class FSLRawDataAsyncWorker;

/**
 * Raw data logger to bson format
 */
class FSLRawDataWriterBson : public ISLRawDataWriter
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
#if USE_LIBMONGO
	// Add actors
	void AddActors(bson_t& OutBsonEntitiesArr);

	// Add components
	void AddComponents(bson_t& OutBsonEntitiesArr);

	// Get entry as Bson object
	bson_t GetAsBsonEntry(const FString& InId,
		const FString& InClass,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Write entry to file
	void WriteData(uint8* memorybuffer, int64 bufferlen);
#endif //USE_LIBMONGO
	// Pointer to worker parent (access to raw data structure)
	FSLRawDataAsyncWorker* WorkerParent;

	// File handle to write the raw data to file
	IFileHandle* FileHandle;
};
