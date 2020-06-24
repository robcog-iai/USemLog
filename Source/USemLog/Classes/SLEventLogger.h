// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "SLOwlExperiment.h"
#include "Events/ISLEventHandler.h"
#if SL_WITH_ROSBRIDGE
#include "ROSBridgeHandler.h"
#include "std_msgs/String.h"
#include "SLROSServiceClient.h"
#endif // SL_WITH_ROSBRIDGE

#include "SLEventLogger.generated.h"

// Forward declaration
class ISLEvent;

/**
* Parameters for creating an event logger
*/
struct FSLEventWriterParams
{
	// Location where to save the data (filename/database name etc.)
	FString TaskId;

	// Episode unique id
	FString EpisodeId;

	// Task description
	//FString TaskDescription;

	// Server ip (optional)
	FString ServerIp;

	// Server Port (optional)
	uint16 ServerPort;

	// Constructor
	FSLEventWriterParams(
		const FString& InTaskId,
		const FString& InEpisodeId,
		//const FString& InTaskDescription,
		const FString& InServerIp = "",
		uint16 InServerPort = 0
	) :
		TaskId(InTaskId),
		EpisodeId(InEpisodeId),
		//TaskDescription(InTaskDescription),
		ServerIp(InServerIp),
		ServerPort(InServerPort)
	{};
};

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
		bool bInPickAndPlaceEvents,
		bool bInLogSlicingEvents,
		bool bInWriteTimelines,
		bool bInLogThroughROS);
	

	// Start logger
	void Start();

	// Finish logger
	void Finish(const float Time, bool bForced = false);

private:
	// Check if the component is valid in the world and has a semantically annotated owner
	bool IsValidAndAnnotated(UActorComponent* Comp) const;
	
	// Called when a semantic event is done
	void OnSemanticEvent(TSharedPtr<ISLEvent> Event);

	// Write events to file
	bool WriteToFile();

	// Log events to KnowRob
	void LogThroughROS(TSharedPtr<ISLEvent> Event);

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

	// Send events through ROS
	bool bLogThroughROS;

	// Array of finished events
	TArray<TSharedPtr<ISLEvent>> FinishedEvents;

	// Owl document of the finished events
	TSharedPtr<FSLOwlExperiment> ExperimentDoc;

	// Semantic event handlers (takes input raw events, outputs finished semantic events)
	TArray<TSharedPtr<ISLEventHandler>> EventHandlers;

	// List of the contact trigger shapes, stored to call Start and Finish on them
	TArray<class ISLContactShapeInterface*> ContactShapes;

	// Cache of the grasp listeners
	TArray<class USLManipulatorListener*> GraspListeners;

	// Cache of the pick and place listeners
	TArray<class USLPickAndPlaceListener*> PickAndPlaceListeners;

	// Cache of the grasp listeners
	TArray<class USLReachListener*> ReachListeners;

	// Cache of the container manipulation listeners
	TArray<class USLContainerListener*> ContainerListeners;

	// ROS Logging
#if SL_WITH_ROSBRIDGE
	TSharedPtr<FROSBridgeHandler> ROSHandler;
	TSharedPtr<SLROSServiceClient> ROSClient;

#endif // SL_WITH_ROSBRIDGE

};
