// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "UObject/NoExportTypes.h"
#include "SLOwlExperiment.h"
#include "Events/ISLEventHandler.h"
#include "SLMetadataWriter.h"
#include "SLEventLogger.generated.h"

// Forward declaration
class ISLEvent;

/**
 * Event (symbolic) data logger
 */
UCLASS()
class USEMLOG_API USLEventLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLEventLogger();

	// Destructor
	~USLEventLogger();

	// Init Logger
	void Init(ESLOwlExperimentTemplate TemplateType,
		const FSLEventWriterParams& WriterParams,
		bool bInLogContactEvents,
		bool bInLogSupportedByEvents,
		bool bInLogGraspEvents,
		bool bInLogSlicingEvents,
		bool bInWriteTimelines,
		bool bInWriteMetadata);

	// Start logger
	void Start();

	// Finish logger
	void Finish(const float Time, bool bForced = false);

private:
	// Called when a semantic event is done
	void OnSemanticEvent(TSharedPtr<ISLEvent> Event);

	// Write events to file
	bool WriteToFile();

	// Create events doc template
	TSharedPtr<FSLOwlExperiment> CreateEventsDocTemplate(
		ESLOwlExperimentTemplate TemplateType, const FString& InDocId);

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
	ESLOwlExperimentTemplate OwlDocTemplate;

	// Save events to timelines
	bool bWriteTimelines;

	// Write metadata (world and events)
	bool bWriteMetadata;

	// Metadata writer
	FSLMetadataWriter MetadataWriter;

	// Array of finished events
	TArray<TSharedPtr<ISLEvent>> FinishedEvents;

	// Owl document of the finished events
	TSharedPtr<FSLOwlExperiment> ExperimentDoc;

	// Semantic event handlers (takes input raw events, outputs finished semantic events)
	TArray<TSharedPtr<ISLEventHandler>> EventHandlers;

	// Cache of the semantic overlap areas
	TArray<class USLOverlapShape*> OverlapShapes;

	// Cache of the grasp listeners
	TArray<class USLGraspListener*> GraspListeners;
};
