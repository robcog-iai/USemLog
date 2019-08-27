// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLPickAndPlaceListener.h"
#include "SLManipulatorListener.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLEntitiesManager.h"
#include "GameFramework/PlayerController.h"


// Sets default values for this component's properties
USLPickAndPlaceListener::USLPickAndPlaceListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	bDetectLiftEvents = true;
	bDetectSlideEvents = true;
	bDetectTransportEvents = true;

	UpdateRate = 0.15f;

	GraspedObject = nullptr;
}

// Dtor
USLPickAndPlaceListener::~USLPickAndPlaceListener()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLPickAndPlaceListener::Init()
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
void USLPickAndPlaceListener::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe for grasp notifications from sibling component
		if(SubscribeForGraspEvents())
		{
			// Start update callback (will directly be paused until a grasp is active)
			GetWorld()->GetTimerManager().SetTimer(
				UpdateTimerHandle, this, &USLPickAndPlaceListener::Update, UpdateRate, true);
			GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle);
			
			// Mark as started
			bIsStarted = true;
		}
	}
}


// Stop publishing grasp events
void USLPickAndPlaceListener::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Subscribe for grasp events from sibling component
bool USLPickAndPlaceListener::SubscribeForGraspEvents()
{
	if(USLManipulatorListener* Sibling = CastChecked<USLManipulatorListener>(
		GetOwner()->GetComponentByClass(USLManipulatorListener::StaticClass())))
	{
		Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLPickAndPlaceListener::OnSLGraspBegin);
		Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLPickAndPlaceListener::OnSLGraspEnd);
		return true;
	}
	return false;
}

// Called when grasp starts
void USLPickAndPlaceListener::OnSLGraspBegin(const FSLEntity& Self, UObject* Other, float Time, const FString& GraspType)
{
	if(GraspedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Multiple objects grasped? TODO"), *FString(__func__), __LINE__);
	}
	
	if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Other))
	{
		GraspedObject = AsSMA;
		for (const auto& C : GraspedObject->GetComponents())
		{
			if(C->Implements<USLContactShapeInterface>())
			{
				GraspedObjectContactShape = Cast<ISLContactShapeInterface>(C);
				if(GraspedObjectContactShape)
				{
					if(GetWorld()->GetTimerManager().IsTimerPaused(UpdateTimerHandle))
					{
						GetWorld()->GetTimerManager().UnPauseTimer(UpdateTimerHandle);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen, timer should have been paused.."), *FString(__func__), __LINE__);
					}
				}
				break; // stop iterating other components
			}
		}
	}
}

// Called when grasp ends
void USLPickAndPlaceListener::OnSLGraspEnd(const FSLEntity& Self, UObject* Other, float Time)
{
	if(Other == GraspedObject)
	{
		GraspedObject = nullptr;
		GraspedObjectContactShape = nullptr;
		if(!GetWorld()->GetTimerManager().IsTimerPaused(UpdateTimerHandle))
		{
			GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen, timer should have been running.."),
				*FString(__func__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen (were there multiple objects grasped?)"),
				*FString(__func__), __LINE__);
	}
}

// Update callback
void USLPickAndPlaceListener::Update()
{
	if(GraspedObject == nullptr || GraspedObjectContactShape == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Thus should not happen....."), *FString(__func__), __LINE__);
		return;
	}
	
	if(GraspedObjectContactShape->IsSupportedBySomething())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %f %s -> supported"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GraspedObject->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %f %s -> flaoting"), *FString(__func__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GraspedObject->GetName());
	}
}
