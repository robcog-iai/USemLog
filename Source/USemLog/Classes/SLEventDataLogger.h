// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OwlExperiment.h"
#include "EventData/SLContactEvent.h"
#include "EventData/SLSupportedByEvent.h"
#include "EventData/SLSlidingEvent.h"
#include "EventData/SLPushedEvent.h"
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
	// Register for semantic contact events
	void ListenToSemanticContactRelatedEvents();

	// Called when a semantic contact is finished
	void OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event);

	// Called when a semantic supported by event is finished
	void OnSemanticSupportedByEvent(TSharedPtr<FSLSupportedByEvent> Event);

	// Write events to file
	bool WriteToFile();

	// Create events doc template
	TSharedPtr<FOwlExperiment> CreateEventsDocTemplate(
		EOwlExperimentTemplate TemplateType, const FString& InDocId);

	// Finish the pending events at the current time
	void FinishPendingEvents(const float EndTime);

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
