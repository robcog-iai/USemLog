// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLItemScanner.h"
#include "ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

// Ctor
USLItemScanner::USLItemScanner()
{
	// Find and set the scanning room mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ScanBoxAsset(
		TEXT("/USemLog/Vision/SM_SLVisScanBox.SM_SLVisScanBox"));
	if(ScanBoxAsset.Succeeded())
	{
		SetStaticMesh(ScanBoxAsset.Object);
		SetMobility(EComponentMobility::Movable);
	}

	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLItemScanner::~USLItemScanner()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();
	}
}

// Init scanner
void USLItemScanner::Init()
{
	if (!bIsInit)
	{
		if(GetStaticMesh())
		{
			// Move the scanning room away from the kitchen
			SetWorldLocation(FVector(0.f,0.f, ZOffset));
			bIsInit = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Scanning room not found, skipping abort scanning.."), *FString(__func__), __LINE__);
		}
	}
}

// Start scanning
void USLItemScanner::Start()
{
	if(!bIsStarted && bIsInit)
	{
		bIsStarted = true;
	}
}

// Finish scanning
void USLItemScanner::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}
