// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLOwl.h"
#include "SLEventData.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USLEventData : public UObject
{
	GENERATED_BODY()

public:
	// Default constructor
	USLEventData();

	// Destructor
	~USLEventData();

	// Initialise logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool Init(const FString InEpisodeId, const FString InLogDirectoryPath);

	// Start logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool Start(const float Timestamp);

	// Finish logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool Finish(const float Timestamp);

	// Write document to file
	UFUNCTION(BlueprintCallable, Category = SL)
	bool WriteToFile();

	// Get document as a string
	UFUNCTION(BlueprintCallable, Category = SL)
	bool GetAsString(FString& Document);

	// Check if the logger is initialised
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsInit() const { return bIsInit; };

	// Check if the logger is started
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsStarted() const { return bIsStarted; };

	// Check if the logger is finished
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsFinished() const { return bIsFinished; };

private:
	// Set document default values
	void SetDefaultValues();

	// Remove document default values
	void RemoveDefaultValues();

	// Start an event
	bool StartEvent();

	// Finish an event
	bool FinishEvent();

	// Insert finished event
	bool InsertFinishedEvent();

	// Start metadata event
	bool StartMetadataEvent(const float Timestamp);

	// Start metadata event
	bool FinishMetadataEvent(const float Timestamp);

	// Terminate all idling events
	bool FinishAllIdleEvents(const float Timestamp);

	// Event logs document as owl representation
	UPROPERTY(EditAnywhere, Category = SL)
	FOwlDocument OwlDocument;

	// Logging directory path
	FString LogDirectoryPath;

	// Id of the log file
	FString EpisodeId;

	// Logger initialised
	uint8 bIsInit : 1;

	// Logger started
	uint8 bIsStarted : 1;

	// Logger finished
	uint8 bIsFinished : 1;

	// Shows if the default values of the owl document have been set
	uint8 bOwlDefaultValuesSet : 1;
};
