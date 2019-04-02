// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLBoneOverlapShape.h"
#if WITH_EDITOR
#include "Components/ArrowComponent.h"
#endif // WITH_EDITOR

// Ctor
USLBoneOverlapShape::USLBoneOverlapShape()
{
#if WITH_EDITORONLY_DATA
	ArrowVis = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("BoneView"));
	if (ArrowVis)
	{
		ArrowVis->ArrowColor = FColor::Red;
		ArrowVis->bTreatAsASprite = true;
		ArrowVis->bLightAttachment = true;
		ArrowVis->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA

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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d"), TEXT(__FUNCTION__), __LINE__);
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