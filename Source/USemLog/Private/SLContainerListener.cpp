// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContainerListener.h"
#include "SLEntitiesManager.h"
#include "SLManipulatorListener.h"

// Sets default values for this component's properties
USLContainerListener::USLContainerListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	CurrGraspedObj = nullptr;
}

// Dtor
USLContainerListener::~USLContainerListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLContainerListener::Init()
{
	if (!bIsInit)
	{
		// Init the semantic entities manager
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(GetWorld());
		}

		// Check that the owner is part of the semantic entities
		SemanticOwner = FSLEntitiesManager::GetInstance()->GetEntity(GetOwner());
		if (!SemanticOwner.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Owner is not semantically annotated.."), *FString(__func__), __LINE__);
			return false;
		}

		bIsInit = true;
		return true;
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLContainerListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if(USLManipulatorListener* Sibling = CastChecked<USLManipulatorListener>(
			GetOwner()->GetComponentByClass(USLManipulatorListener::StaticClass())))
		{
			Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLContainerListener::OnSLGraspBegin);
			Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLContainerListener::OnSLGraspEnd);

			bIsStarted = true;
		}
	}
}

// Finish active events
void USLContainerListener::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Finish any active event
		FinishActiveEvents();

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}


// Called when grasp starts
void USLContainerListener::OnSLGraspBegin(const FSLEntity& Self, AActor* Other, float Time, const FString& GraspType)
{
	if(CurrGraspedObj)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Cannot set %s as grasped object.. manipulator is already grasping %s;"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
		return;
	}
	else
	{
		CurrGraspedObj = Other;
		SetContainersAndDistances();
	}
}

// Called when grasp ends
void USLContainerListener::OnSLGraspEnd(const FSLEntity& Self, AActor* Other, float Time)
{
	if(CurrGraspedObj == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.. currently grasped object is nullptr while ending grasp with %s"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
		return;
	}

	if(CurrGraspedObj == Other)
	{
		CurrGraspedObj = nullptr;

		// Check which events to send
		Containers.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] End grasp with %s while %s is still grasped.. ignoring event.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
	}
}

// Search which container will be manipulated and save their current distance to the grasped item
bool USLContainerListener::SetContainersAndDistances()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d CurrGraspedObj %s, Outermost=%s.."),
		*FString(__func__), __LINE__, *CurrGraspedObj->GetName(), *CurrGraspedObj->GetOutermost()->GetName());
	return true;
}

// Finish any active events
void USLContainerListener::FinishActiveEvents()
{
}
