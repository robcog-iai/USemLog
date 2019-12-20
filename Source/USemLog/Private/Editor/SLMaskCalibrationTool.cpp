// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLMaskCalibrationTool.h"

// Ctor
USLMaskCalibrationTool::USLMaskCalibrationTool()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLMaskCalibrationTool::~USLMaskCalibrationTool()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();
	}
}


// Init scanner
void USLMaskCalibrationTool::Init()
{
	if (!bIsInit)
	{
		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLMaskCalibrationTool::Start()
{
	if (!bIsStarted && bIsInit)
	{
		bIsStarted = true;
	}
}

// Finish scanning
void USLMaskCalibrationTool::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}