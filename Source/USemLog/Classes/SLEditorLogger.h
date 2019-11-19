// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "UObject/NoExportTypes.h"
#include "SLEditorLogger.generated.h"

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
	void Init(const FString& InTaskId);

	// Start logger
	void Start(
		bool bWriteSemanticMap,
		bool bClearTags,
		const FString& ClearTagType,
		const FString& ClearKeyType,
		bool bOverwriteProperties,
		bool bWriteClassProperties,
		bool bWriteUniqueIdProperties,
		bool bWriteVisualMaskProperties,
		int32 VisualMaskColorMinDistance,
		bool bRandomMaskGenerator);
	
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

	// Task id
	FString TaskId;
	
};
