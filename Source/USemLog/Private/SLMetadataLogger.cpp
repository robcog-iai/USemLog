// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMetadataLogger.h"
#include "SLEntitiesManager.h"
#include "Tags.h"
#include "Components/SkeletalMeshComponent.h"

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

	// Disconnect and clean db connection
	DBHandler.Disconnect();
}

// Init logger
void USLMetadataLogger::Init(const FString& InTaskId, const FString& InServerIp, uint16 InServerPort, 
	bool bOverwrite, bool bScanItems, FSLMetaScannerParams ScanParams)
{
	if (!bIsInit)
	{
		// Check that the virtual camera class names are not empty or duplicates
		FSLEntitiesManager::GetInstance()->Init(GetWorld());
		if(FSLEntitiesManager::GetInstance()->EmptyOrDuplicatesInTheCameraViews())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Empty or duplicate class names within the vision cameras, aborting.."), *FString(__func__), __LINE__);
			return;
		}

		// Connect to the db
		if (!DBHandler.Connect(InTaskId, InServerIp, InServerPort, bOverwrite))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not connect to the db.."), *FString(__func__), __LINE__);
			return;
		}

		if(bScanItems)
		{
			ItemsScanner = NewObject<USLMetaScanner>(this);
			ItemsScanner->Init(InTaskId, ScanParams);
		}
		bIsInit = true;
	}
}

// Start logger
void USLMetadataLogger::Start(const FString& InTaskDescription)
{
	if (!bIsStarted && bIsInit)
	{
		DBHandler.WriteFirstDocument(InTaskDescription);

		// Start the item scanner, for every finished scan it will trigger an update call on the logger
		if (ItemsScanner)
		{
			ItemsScanner->Start();
		}

		bIsStarted = true;
	}
}

// Finish logger
void USLMetadataLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if(ItemsScanner)
		{
			ItemsScanner->Finish();
		}

		DBHandler.CreateIndexes();
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Create the scan entry bson document
void USLMetadataLogger::StartScanEntry(const FString& Class, int32 ResX, int32 ResY)
{
	DBHandler.StartScanEntry(Class, ResX, ResY);
}

// Add pose scan data
void USLMetadataLogger::AddScanPoseEntry(const FSLScanPoseData& ScanPoseData)
{
	DBHandler.AddScanPoseEntry(ScanPoseData);
}

// Write and clear the scan entry to the database
void USLMetadataLogger::FinishScanEntry()
{
	DBHandler.FinishScanEntry();
}
