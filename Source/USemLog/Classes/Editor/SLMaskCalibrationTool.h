// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLMaskCalibrationTool.generated.h"

/**
 * Renders the masked materials on the items and 
 * compares the rendered pixel values with the original ones
 */
UCLASS()
class USLMaskCalibrationTool : public UObject
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLMaskCalibrationTool();

	// Dtor
	~USLMaskCalibrationTool();

	// Setup scanning room
	void Init();

	// Start scanning, set camera into the first pose and trigger the screenshot
	void Start();

	// Finish scanning
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Set when initialized
	bool bIsInit;

	// Set when started
	bool bIsStarted;

	// Set when finished
	bool bIsFinished;
};
