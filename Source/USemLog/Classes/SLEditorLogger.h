// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "UObject/NoExportTypes.h"
#include "Editor/SLEditorStructs.h"
#include "SLEditorLogger.generated.h"

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
	void Init(const FString& InTaskId, const FString& ServerIp,
		uint16 ServerPort, ESLAssetAction InAction = ESLAssetAction::NONE, bool bCalibrateRenderedMaskColors = false, bool bMaskColorsOnlyDemo = false, bool bOverwrite = false);

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
