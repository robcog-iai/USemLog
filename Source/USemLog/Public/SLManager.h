// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "USemLog.h"
#include "GameFramework/Info.h"
#include "Runtime/SLWorldStateLogger.h"
#include "SLWorldLogger.h"
#include "SLEventLogger.h"
#include "SLMetadataLogger.h"
#include "SLVisionLogger.h"
#include "SLEditorLogger.h"
#include "SLDataVisualizer.h"
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

	// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
	virtual void PostLoad() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	//// Called by the editor to query whether a property of this object is allowed to be modified.
	//virtual bool CanEditChange(const UProperty* InProperty) const override;
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

	// Get TaskId
	FString GetTaskId() const { return TaskId; };

	// Get episode id
	FString GetEpisodeId() const { return EpisodeId; };

private:
	// Setup user input bindings
	void SetupInputBindings();

	// Start/finish logger from user input
	void UserInputToggleCallback();

private:
	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

	/*******************************************************************************************************/
	/* Semantic logger */
	/******************************************************************************************************/
	// Log directory (or the database name if saving to mongodb)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString TaskId;

	// Set to true in order to edit the episode id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomEpisodeId;

	// Episode Id (be default will be auto generated)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseCustomEpisodeId"))
	FString EpisodeId;

	// Task description
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString TaskDescription;

	// Reset start time to 0 when starting to log
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bResetStartTime;
	
	// Start at load time
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtBeginPlay;

	// Start after begin play, in the first tick 
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtNextTick;

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
	FName UserInputActionName;

	// Mongodb server IP
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString ServerIp;

	// Mongodb server PORT
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (ClampMin = 0, ClampMax = 65535))
	uint16 ServerPort;

	/******************************************************************************************************/
	/* Begin task Metadata Logger properties */
	/******************************************************************************************************/
	// Include task related metadata
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogMetadata;

	// Overwrite existing entries
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bLogMetadata"))
	bool bOverwriteMetadata;

	// Perform a 3d sphere image scan of all the handheld items
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bLogMetadata"))
	bool bScanItems;

	// Scan image resolution
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bScanItems"))
	FIntPoint ScanResolution;

	// Number of camera poses on the sphere pointed toward the object
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bScanItems"))
	int32 NumberOfScanPoints;

	// The maximum volume (cm^3) of an item that should be scanned (0 = no limit)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bScanItems"))
	float MaxScanItemVolume;

	// The distance of the camera to the scanned item (0 = automatically calculated relative to the item size)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bScanItems"))
	float CameraDistanceToScanItem;
	
	// Save the scanned images locally
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Metadata Logger", meta = (editcondition = "bScanItems"))
	bool bIncludeScansLocally;
	
	// Metadata logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLMetadataLogger* MetadataLogger;
	/* End task metadata logger properties */
	

	/******************************************************************************************************/
	/* Begin World State Logger properties */
	/******************************************************************************************************/
	// Log world state
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogWorldState;

	// Remove any existing entries
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	bool bOverwriteWorldState;

	// Update rate (s) of world state logging (0.f means logging on every tick) (not fixed nor guaranteed)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"), meta = (ClampMin = 0))
	float WorldStateUpdateRate;

	// Distance (cm) threshold difference for logging a given item
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"), meta = (ClampMin = 0))
	float LinearDistance;

	// Rotation (radians) threshold difference for logging a given item
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"), meta = (ClampMin = 0))
	float AngularDistance;

	// Writer type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	ESLWorldWriterType WriterType;

	// World state logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLWorldLogger* WorldStateLogger;
	/* End world state logger properties */


	/******************************************************************************************************/
	/* Begin Event Data Logger properties */
	/******************************************************************************************************/
	// Log event data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogEventData;

	// Listen for contact events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogContactEvents;

	// Listen for supported by events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogSupportedByEvents;
	
	// Listen for grasping events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogGraspEvents;

	// Listen for PickAndPlace events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogPickAndPlaceEvents;

	// Listen for slicing events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bLogSlicingEvents;

	// Write event data as timelines
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bWriteTimelines;

	// Includes the related events in the episode (TODO)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bWriteEpisodeMetadata;

	// Logging through ROSBridge
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	bool bTurnOffROSLogging;

	// Owl experiment template
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger", meta = (editcondition = "bLogEventData"))
	ESLOwlExperimentTemplate ExperimentTemplateType;

	// Event data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLEventLogger* EventDataLogger;
	/* End event data logger properties */


	/******************************************************************************************************/
	/* Begin Vision Data Logger properties */
	/******************************************************************************************************/
	// Log vision data (this will cause the episode to be replayed for logging image data)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogVisionData;

	// Remove any previously existing entries from the collection
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"))
	bool bOverwriteVisionData;

	// Resolution of the images
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"), meta = (ClampMin = 1))
	FIntPoint VisionImageResolution;

	// Calculate overlaps of every object in the scene
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"))
	bool bCalculateOverlaps;

	// Make screenshots for calculating overlaps smaller for faster logging
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"))
	uint8 OverlapResolutionDivisor;
	
	// Update rate of the vision logger (0 - updates at every available frame)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"), meta = (ClampMin = 0))
	float VisionUpdateRate;

	// Store the images locally in the task folder
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Vision Data Logger", meta = (editcondition = "bLogVisionData"))
	bool bIncludeImagesLocally;
	
	// Vision data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLVisionLogger* VisionDataLogger;
	/* End vision data logger properties */


	/******************************************************************************************************/
	/* Begin Editor Logger properties */
	/******************************************************************************************************/
	// Log editor related data, workaround to access actors from sublevels
	// (GEditor->GetEditorWorldContext().World() does not seem to store data on the sublevels)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogEditorData;

	// Overwrite existing editor related entries (where applicable)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bOverwriteEditorData;

	// Save data to an owl semantic map
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bWriteSemanticMap;

	// Clear all tags (if ClearOnlyThisTagType is empty)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bClearAllTags;

	// Clear only tags with this tag type (empty means clear all tags everything)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bClearTags"))
	FString TagTypeToClear;

	// Remove only the given key from the tag type (if empty, it removes all keys)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bClearTags"))
	FString TagKeyToClear;

	
	// Add class property to tags
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bWriteClassTags;

	// Add unique id property to tags
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bWriteUniqueIdTags;
	
	// Add visual mask data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bWriteUniqueMaskColors;

	// Min distance between the mask colors
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bWriteVisualMaskProperty"))
	uint8 MinColorManhattanDistance;
	
	// Generate visual masks randomly or incrementally
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bWriteVisualMaskProperty"))
	bool bUseRandomColorGeneration;

	// Tag non-movable entities
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bWriteVisualMaskProperty"))
	bool bWriteNonMovableTags;

	// Calibrate the rendered mask colors (there is a difference between the actual value and the rendered pixel values)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	bool bCalibrateRenderedMaskColors;

	// Set the calibration renderer to only show the visual masks as a demo
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bCalibrateRenderedMaskColors"))
	bool bMaskColorsOnlyDemo;

	// Calibrate the rendered mask colors (there is a difference between the actual value and the rendered pixel values)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Editor Logger", meta = (editcondition = "bLogEditorData"))
	ESLAssetAction EditorAssetAction;
	
	// Vision data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLEditorLogger* EditorLogger;
	/* End vision data logger properties */


	/******************************************************************************************************/
	/* Begin Data Visualizer properties */
	/******************************************************************************************************/
	// Visualize logged data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bVisualizeData;

	// Viz queries to execute
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Data Visualizer", meta = (editcondition = "bVisualizeData"))
	TArray<class USLDataVisQueries*> VisQueriesArray;

	// Camera to SetViewTarget to when starting the world (default pose if not set)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Data Visualizer", meta = (editcondition = "bVisualizeData"))
	class ACameraActor* CameraViewTarget;

	// Data visualizer, use UPROPERTY to avoid GC
	UPROPERTY()
	USLDataVisualizer* DataVisualizer;
	/* End data visualizer properties */



	/***/
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger|Loggers")
	ASLWorldStateLogger* ActualWorldStateLogger;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Loggers")
	FSLWorldStateLoggerParams ActualWorldStateLoggerParams;
};