// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "USemLog.h"
#include "GameFramework/Info.h"
#include "SLWorldStateLogger.h"
#include "SLEventLogger.h"
#include "SLManager.generated.h"

/**
 * Semantic logging manager (controls the logging in the world)
 */
UCLASS(ClassGroup = (SL), hidecategories = (LOD, Cooking, Transform), DisplayName="SL Manager" )
class USEMLOG_API ASLManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLManager();

	// Destructor
	~ASLManager();

protected:
	// Allow actors to initialize themselves on the C++ side
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// Called by the editor to query whether a property of this object is allowed to be modified.
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif // WITH_EDITOR

public:
	// Init loggers
	void Init();

	// Start loggers
	void Start();

	// Finish loggers (forced if called from destructor)
	void Finish(const float Time, bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

private:
	// Setup user input bindings
	void SetupInputBindings();

	// Call start from user input
	void StartFromInput();

	// Call finish from user input
	void FinishFromInput();

private:
	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

	/* Semantic logger */
	// Set to true in order to edit the episode id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomEpisodeId;

	// Episode Id (be default will be auto generated)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseCustomEpisodeId"))
	FString EpisodeId;

	// Log directory (or the database name if saving to mongodb)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString Location;

	// Start at load time
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtBeginPlay;

	// Start after begin play, in the first tick
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtFirstTick;

	// Start after a given delay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartWithDelay;

	// Start after a given delay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartWithDelay"))
	float StartDelay;

	// Start from external user input
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartFromUserInput;

	// Action name for starting from user input
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartFromUserInput"))
	FName StartInputActionName;

	// Action name for finishing from user input
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartFromUserInput"))
	FName FinishInputActionName;

	/* Begin world state logger properties */
	// Log world state
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogWorldState;

	// Update rate (ms) of world state logging (0.f means logging on every tick) (not fixed nor guaranteed)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"), meta = (ClampMin = 0))
	float UpdateRate;

	// Distance (cm) threshold difference for logging a given item
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"), meta = (ClampMin = 0))
	float DistanceStepSize;

	// Rotation (radians) threshold difference for logging a given item
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"), meta = (ClampMin = 0))
	float RotationStepSize;

	// Writer type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	ESLWorldStateWriterType WriterType;

	// Mongodb server IP
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger")
	FString HostIP;

	// Mongodb server PORT
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (ClampMin = 0, ClampMax = 65535))
	uint16 HostPort;

	// World state logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLWorldStateLogger* WorldStateLogger;
	/* End world state logger properties */

	/* Begin event data logger properties */
	// Log event data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogEventData;

	// Listen for contact events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogContactEvents;

	// Listen for supported by events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogSupportedByEvents;

	//// Listen for sliding events
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	//bool bLogSlidingEvents;

	//// Listen for pushed events
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	//bool bLogPushedEvents;
	
	// Listen for grasping events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogGraspEvents;

	// Write event data as timelines
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bWriteTimelines;

	// Owl experiment template
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	ESLOwlExperimentTemplate ExperimentTemplateType;

	// Event data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLEventLogger* EventDataLogger;
	/* End event data logger properties */

	/* Begin vision data logger properties */
	// Log vision data (this will cause the episode to be replayed for logging image data)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger")
	bool bLogVisionData;

	// Maximum number of demo frames recorded per second
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"), meta = (ClampMin = 0))
	float MaxRecordHz;

	// Minimum number of demo frames recorded per second
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"), meta = (ClampMin = 0))
	float MinRecordHz;
	/* End vision data logger properties */
};
