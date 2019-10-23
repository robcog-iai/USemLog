// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMetadataLogger.h"



// Ctor
USLMetadataLogger::USLMetadataLogger()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLMetadataLogger::~USLMetadataLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Init logger
void USLMetadataLogger::Init(const FString& InLocation, const FString InEpisodeId, const FString InServerIp, uint16 InServerPort,
		const FString& InTaskDescription,  bool bWriteItemImageScans)
{
	if (!bIsInit)
	{
		bIsInit = true;
	}
}

// Start logger
void USLMetadataLogger::Start()
{
	if (!bIsStarted && bIsInit)
	{
		bIsStarted = true;
	}
}

// Finish logger
void USLMetadataLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Connect to the database
bool USLMetadataLogger::Connect(const FString& DBName, const FString& ServerIp, uint16 ServerPort)
{
}


