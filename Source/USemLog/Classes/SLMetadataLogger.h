// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "UObject/NoExportTypes.h"
#include "Meta/SLMetaScanner.h"
#include "Meta/SLMetaDBHandler.h"
#include "SLMetadataLogger.generated.h"

/**
 * Writes task and episode related metadata
 */
UCLASS()
class USEMLOG_API USLMetadataLogger : public UObject
{
	GENERATED_BODY()

	// Give access to private methods
	friend class USLMetaScanner;
	
public:
	// Ctor
	USLMetadataLogger();

	// Dtor
	~USLMetadataLogger();
	
	// Init logger
	void Init(const FString& InTaskId, const FString& InServerIp, uint16 InServerPort,
		bool bOverWrite = false, bool bScanItems = false, FSLMetaScannerParams ScanParams = FSLMetaScannerParams());

	// Start logger
	void Start(const FString& InTaskDescription);

	// Finish logger
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Create the scan entry bson document (a scan entry contains all the scan images of a given class)
	void StartScanEntry(const FString& Class, int32 ResX, int32 ResY);

	// Add pose scan data
	void AddScanPoseEntry(const FSLScanPoseData& ScanPoseData);
	
	// Write and clear the scan entry to the database
	void FinishScanEntry();
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Helper class for scanning the items from the world
	UPROPERTY() // Avoid GC
	USLMetaScanner* ItemsScanner;

	// Database handler
	FSLMetaDBHandler DBHandler;
};
