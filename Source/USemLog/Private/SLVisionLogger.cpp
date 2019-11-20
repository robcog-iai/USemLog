// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionLogger.h"
#include "SLEntitiesManager.h"

// Constructor
USLVisionLogger::USLVisionLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
}

// Destructor
USLVisionLogger::~USLVisionLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Init Logger
void USLVisionLogger::Init(const FString& InTaskId, const FString& InEpisodeId, const FString& InServerIp, uint16 InServerPort, float InUpdateRate)
{
	if (!bIsInit)
	{

	}
}

// Start logger
void USLVisionLogger::Start(const FString& EpisodeId)
{
	if (!bIsStarted && bIsInit)
	{
		// Mark as started
		bIsStarted = true;
	}
}

// Finish logger
void USLVisionLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}
