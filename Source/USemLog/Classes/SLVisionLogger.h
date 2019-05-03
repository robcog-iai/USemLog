// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisionLogger.generated.h"

/**
* Vision logger parameters
*/
struct FSLVisionLoggerParams
{
};

/**
 * Vision logger, can only be used with USemLogVision module
 * it saves the episode as a replay
 */
UCLASS()
class USLVisionLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLVisionLogger();

	// Destructor
	~USLVisionLogger();

	// Init Logger
	void Init(float InMaxRecHz, float InMinRecHz);

	// Start logger
	void Start(const FString& EpisodeId);

	// Finish logger
	void Finish(bool bForced = false);

private:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;
};
