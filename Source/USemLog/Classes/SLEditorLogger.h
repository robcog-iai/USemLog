// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "UObject/NoExportTypes.h"
#include "SLEditorLogger.generated.h"

UENUM()
enum class ESLAssetAction : uint8
{
	NONE				UMETA(DisplayName = "NONE"),
	Download			UMETA(DisplayName = "Download Assets"),
	Move				UMETA(DisplayName = "Move Assets"),
	Upload				UMETA(DisplayName = "Upload Assets"),
	MoveAndUpload		UMETA(DisplayName = "Move then Upload Assets"),
};

/*
* Editor logger parameters
*/
struct FSLEditorLoggerParams
{
	// Default ctor
	FSLEditorLoggerParams() : 
		bOverwrite(false),
		bWriteSemanticMap(false),
		bClearAllTags(false),
		bWriteClassTags(false),
		bWriteUniqueIdTags(false),
		bWriteUniqueMaskColors(false),
		MinColorManhattanDistance(17),
		bUseRandomColorGeneration(true)
	{};

	// Init ctor
	FSLEditorLoggerParams(bool bNewOverwrite,
		bool bNewWriteSemanticMap,
		bool bNewClearAllTags,
		const FString& InTagTypeToClear,
		const FString& InTagKeyToClear,
		bool bNewWriteClassTags,
		bool bNewWriteUniqueIdTags,
		bool bNewWriteUniqueMaskColors,
		uint8 NewMinColorManhattanDistance,
		bool bNewUseRandomColorGeneration)
		:
		bOverwrite(bNewOverwrite),
		bWriteSemanticMap(bNewWriteSemanticMap),
		bClearAllTags(bNewClearAllTags),
		TagTypeToClear(InTagTypeToClear),
		TagKeyToClear(InTagKeyToClear),
		bWriteClassTags(bNewWriteClassTags),
		bWriteUniqueIdTags(bNewWriteUniqueIdTags),
		bWriteUniqueMaskColors(bNewWriteUniqueMaskColors),
		MinColorManhattanDistance(NewMinColorManhattanDistance),
		bUseRandomColorGeneration(bNewUseRandomColorGeneration)
	{};

	// Overwrite the data (where applicable)
	bool bOverwrite;

	// Write the semantic map
	bool bWriteSemanticMap;

	/* Tag related */
	// Clear all tags in the world
	bool bClearAllTags;

	// Clear only the given tag type (e.g. SemLog)
	FString TagTypeToClear;

	// Clear only the given tag keys (e.g. Class)
	FString TagKeyToClear;

	// Write class properties to tags
	bool bWriteClassTags;

	// Write unique ids to tags
	bool bWriteUniqueIdTags;

	/* Mask color generation */
	// Write unique color masks
	bool bWriteUniqueMaskColors;

	// Guaranteed minimal manhattan distance between the generated mask colors
	uint8 MinColorManhattanDistance;

	// Algorithm to generate the colors (random color generation, or incrementally fill an array and pick random indexes)
	bool bUseRandomColorGeneration;
};

// Forward declarations
class USLMaskCalibrationTool;
class USLAssetManager;

/**
 * Workaround to use the editor functionalities including the sublevel actors
 */
UCLASS()
class USLEditorLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLEditorLogger();

	// Destructor
	~USLEditorLogger();

	// Init Logger
	void Init(const FString& InTaskId, bool bCalibrateRenderedMaskColors = false, ESLAssetAction AssetAction = ESLAssetAction::NONE);

	// Start logger
	void Start(const FSLEditorLoggerParams& InParams);
	
	// Finish logger
	void Finish(bool bForced = false);

private:
	// Safely quit the editor
	void QuitEditor();
	
private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;

	// Mask calibration tool 
	// there is a difference between the given mask color, and the rendered one
	UPROPERTY() // Avoid GC
	USLMaskCalibrationTool* CalibrationTool;

	// Used for downloading / uploading the assets used in the current task
	UPROPERTY()
	USLAssetManager* AssetManager;


	// Task id
	FString TaskId;
	
};
