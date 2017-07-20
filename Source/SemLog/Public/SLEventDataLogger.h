// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLOwl.h"
#include "SLEventDataLogger.generated.h"


/** Delegate type for the finished events */
DECLARE_MULTICAST_DELEGATE_OneParam(FSLOnEventsFinishedSignature, const FString&);

/**
* Semantic logger of event data
* (important contacts, various high level events etc.)
 */
UCLASS()
class SEMLOG_API USLEventDataLogger : public UObject
{
	GENERATED_BODY()

public:
	// Default constructor
	USLEventDataLogger();

	// Destructor
	~USLEventDataLogger();

	// Initialize logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool InitLogger(const FString InEpisodeId);

	// Start logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool StartLogger(const float Timestamp);

	// Finish logger
	UFUNCTION(BlueprintCallable, Category = SL)
	bool FinishLogger(const float Timestamp);

	// Write document to file
	UFUNCTION(BlueprintCallable, Category = SL)
	bool WriteEventsToFile(const FString InLogDirectoryPath);

	// Broadcast document
	UFUNCTION(BlueprintCallable, Category = SL)
	bool BroadcastFinishedEvents();

	// Get document as a string
	UFUNCTION(BlueprintCallable, Category = SL)
	bool GetEventsAsString(FString& OutStringDocument);

	// Check if the logger is initialized
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsInit() const { return bIsInit; };

	// Check if the logger is started
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsStarted() const { return bIsStarted; };

	// Check if the logger is finished
	UFUNCTION(BlueprintCallable, Category = SL)
	bool IsFinished() const { return bIsFinished; };
	
	// Insert finished event
	//UFUNCTION(BlueprintCallable, Category = SL)
	bool InsertFinishedEvent(
		const TSharedPtr<FOwlIndividualName> EventIndividualName,
		const float StartTime,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event
	//UFUNCTION(BlueprintCallable, Category = SL)
	bool StartAnEvent(
		const TSharedPtr<FOwlIndividualName> EventIndividualName,
		const float StartTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Finish an event
	//UFUNCTION(BlueprintCallable, Category = SL)
	bool FinishAnEvent(
		const TSharedPtr<FOwlIndividualName> EventIndividualName,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Delegate to publish the finished events
	FSLOnEventsFinishedSignature OnEventsFinished;

private:
	// Set document default values
	void SetDefaultValues();

	// Remove document default values
	void RemoveDefaultValues();

	// Start metadata event
	bool StartMetadataEvent(const float Timestamp);

	// Start metadata event
	bool FinishMetadataEvent(const float Timestamp);

	// Terminate all idling events
	bool FinishOpenedEvents(const float Timestamp);

	// Event logs document as owl representation
	UPROPERTY(EditAnywhere, Category = SL)
	FOwlDocument OwlDocument;

	// Id of the log file
	FString EpisodeId;

	// Logger initialized
	uint8 bIsInit : 1;

	// Logger started
	uint8 bIsStarted : 1;

	// Logger finished
	uint8 bIsFinished : 1;

	// Shows if the default values of the owl document have been set
	uint8 bOwlDefaultValuesSet : 1;

	// Metadata event
	FOwlNode MetaEvent;

	// Array of all the finished events
	TArray<FOwlNode> FinishedEvents;

	// Map of opened events
	TMap<TSharedPtr<FOwlIndividualName>, FOwlNode> NameToOpenedEvent;

	// TODO AddUnqiue, or using TSet; since time individuals might repeat;
	TArray<FOwlNode> ObjectIndividuals;
};
