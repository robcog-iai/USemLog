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
	bool WriteEventsToFile(const FString InLogDirectoryPath, bool bWriteTimelines = true);

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
	bool InsertFinishedEvent(const TSharedPtr<FOwlNode> Event);

	// Start an event
	bool StartAnEvent(const TSharedPtr<FOwlNode> Event);

	// Finish an event
	bool FinishAnEvent(const TSharedPtr<FOwlNode> Event);

	// Add object individual
	bool AddObjectIndividual(const FString Id, TSharedPtr<FOwlNode> Object);

	// Add time individual
	bool AddTimeIndividual(const FString Id, TSharedPtr<FOwlNode> Object);

	// Add metadata property
	bool AddMetadataProperty(TSharedPtr<FOwlTriple> Property);

	// Delegate to publish the finished events
	FSLOnEventsFinishedSignature OnEventsFinished;

private:
	// Start metadata event
	bool StartMetadataEvent(const float Timestamp);

	// Start metadata event
	bool FinishMetadataEvent(const float Timestamp);

	// Terminate all idling events
	bool FinishOpenedEvents(const float Timestamp);

	// @TODO Temp solution
	// Set objects, time events and metadata subActions
	void SetObjectsAndMetaSubActions();

	// Write timelines
	void WriteTimelines(const FString LogDirectoryPath);

	// Set document default values
	void SetDefaultValues();

	// Remove document default values
	void RemoveDefaultValues();

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
	TSharedPtr<FOwlNode> MetaEvent;

	// Array of all the finished events
	TArray<TSharedPtr<FOwlNode>> FinishedEvents;

	// Set of opened events
	TSet<TSharedPtr<FOwlNode>> OpenedEvents;

	// Map id to object individuals
	TMap <FString, TSharedPtr<FOwlNode>> ObjectIndividualsMap;

	// Map of id to time individuals
	TMap <FString, TSharedPtr<FOwlNode>> TimeIndividualsMap;
};
