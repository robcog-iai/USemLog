// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OwlExperiment.h"
#include "EventData/Events.h"
#include "SLEventDataLogger.generated.h"

// Forward declaration
class ISLEvent;

/**
 * Event (symbolic) data logger
 */
UCLASS()
class USEMLOG_API USLEventDataLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLEventDataLogger();

	// Destructor
	~USLEventDataLogger();

public:
	// Init Logger
	void Init(const FString& InLogDirectory,
		const FString& InEpisodeId,
		EOwlExperimentTemplate TemplateType,
		bool bInWriteTimelines);

	// Start logger
	void Start();

	// Finish logger
	void Finish();

private:
	// Register for various semantic events
	void ListenToSemanticEvents();

	// Called when a semantic contact is finished
	void OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event);

	// Called when a semantic supported by event is finished
	void OnSemanticSupportedByEvent(TSharedPtr<FSLSupportedByEvent> Event);

	// Called when a grasp is finished
	void OnSemanticGraspEvent(TSharedPtr<FSLGraspEvent> Event);

	// Write events to file
	bool WriteToFile();

	// Create events doc template
	TSharedPtr<FOwlExperiment> CreateEventsDocTemplate(
		EOwlExperimentTemplate TemplateType, const FString& InDocId);

	// Finish the pending events at the current time
	void FinishPendingEvents(const float EndTime);

private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Directory where to log
	FString LogDirectory;

	// Unique id of the episode
	FString EpisodeId;

	// Type of owl template to write the events to
	EOwlExperimentTemplate OwlDocTemplate;

	// Save events to timelines
	bool bWriteTimelines;

	// Array of pending events
	TArray<TSharedPtr<ISLEvent>> PendingEvents;

	// Array of pending events
	TArray<TSharedPtr<ISLEvent>> FinishedEvents;

	// Owl document of the finished events
	TSharedPtr<FOwlExperiment> ExperimentDoc;
};
