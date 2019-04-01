// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLBoneOverlapShape.h"

// Ctor
USLBoneOverlapShape::USLBoneOverlapShape()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
}

// Dtor
USLBoneOverlapShape::~USLBoneOverlapShape()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
}

// Attach to bone 
bool USLBoneOverlapShape::Init(USkeletalMeshComponent* Parent, FName BoneName)
{
	if (!bIsInit)
	{
		bIsInit = true;
		return bIsInit;
	}
	return true;
}

// Start listening to overlaps 
void USLBoneOverlapShape::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Mark as started
		bIsStarted = true;
	}
}

// Stop publishing overlap events
void USLBoneOverlapShape::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}