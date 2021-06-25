// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Runtime/SLLoggerStructs.h"
#include "Events/ISLEventHandler.h"
#include "ROSProlog/SLPrologClient.h"
#include "Owl/SLOwlExperiment.h"
#include "SLSymbolicLogger.generated.h"

// Forward declarations
class ASLIndividualManager;

/**
 * Subsymbolic data logger
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Symbolic Logger")
class USEMLOG_API ASLSymbolicLogger : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLSymbolicLogger();

	// Force call on finish
	~ASLSymbolicLogger();

protected:
	// Allow actors to initialize themselves on the C++ side
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Init logger (called when the logger is synced externally)
	void Init(const FSLSymbolicLoggerParams& InLoggerParameters, const FSLLoggerLocationParams& InLocationParameters);

	// Start logger (called when the logger is synced externally)
	void Start();

	// Finish logger (called when the logger is synced externally) (bForced is true if called from destructor)
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Check if the manager is running independently
	bool IsRunningIndependently() const { return bUseIndependently; };

protected:
	// Init logger (called when the logger is used independently)
	void InitImpl();

	// Start logger (called when the logger is used independently)
	void StartImpl();

	// Finish logger (called when the logger is used independently) (bForced is true if called from destructor)
	void FinishImpl(bool bForced = false);

	// Setup user input bindings
	void SetupInputBindings();

	// Start/finish logger from user input
	void UserInputToggleCallback();

	// Called when a semantic event is done
	void SemanticEventFinishedCallback(TSharedPtr<ISLEvent> Event);
	
	// Write data to file
	void WriteToFile();

	// Create events doc template
	TSharedPtr<FSLOwlExperiment> CreateEventsDocTemplate(
		ESLOwlExperimentTemplate TemplateType, const FString& InDocId);

private:
	// Get the reference or spawn a new initialized individual manager
	bool SetIndividualManager();

	// Helper function which checks if the individual data is loaded
	bool IsValidAndLoaded(AActor* Actor);

	// Iterate and init the contact monitors in the world
	void InitContactMonitors();

	// Iterate and init the manipulator contact monitors
	void InitManipulatorContactAndGraspMonitors();

	// Iterate and init the manipulator fixation monitors
	void InitManipulatorGraspFixationMonitors();

	// Iterate and init the manipulator reach monitors
	void InitReachAndPreGraspMonitors();

	// Iterate and init the manipulator container monitors
	void InitManipulatorContainerMonitors();

	// Iterate and init the pick and place monitors
	void InitPickAndPlaceMonitors();

	// Iterate and init the slicing monitors
	void InitSlicingMonitors();

	// Publish data through ROS
	void InitROSPublisher();

protected:
	// True when ready to log
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True when active
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsStarted : 1;

	// True when done logging
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsFinished : 1;
	 
private:
	// If true the logger will start on its own (instead of being started by the manager)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bUseIndependently : 1;

	// Logger parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLSymbolicLoggerParams LoggerParameters;

	// Location parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerLocationParams LocationParameters;

	// Logger start parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerStartParams StartParameters;

	// Access to all individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;


	// Array of finished events
	TArray<TSharedPtr<ISLEvent>> FinishedEvents;

	// Owl document of the finished events
	TSharedPtr<FSLOwlExperiment> ExperimentDoc;

	// Semantic event handlers (takes input raw events, outputs finished semantic events)
	TArray<TSharedPtr<ISLEventHandler>> EventHandlers;

	// List of the contact trigger shapes, stored to call Start and Finish on them
	TArray<class ISLContactMonitorInterface*> ContactMonitors;

	// Cache of the grasp Monitors
	TArray<class USLReachAndPreGraspMonitor*> ReachAndPreGraspMonitors;

	// Cache of the grasp Monitors
	TArray<class USLManipulatorMonitor*> ManipulatorContactAndGraspMonitors;

	// Cache of the pick and place Monitors
	TArray<class USLPickAndPlaceMonitor*> PickAndPlaceMonitors;

	//// Cache of the container manipulation Monitors
	//TArray<class USLContainerMonitor*> ContainerMonitors;

	// Episode start time
	float EpisodeStartTime;

	// Episode end time
	float EpisodeEndTime;

	// ROS publisher
	UPROPERTY()
	USLPrologClient* ROSPrologClient;
};
