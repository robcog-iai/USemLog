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
	
	GraspedObject = nullptr;
	EventCheck = ESLPaPStateCheck::NONE;
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
			GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &USLPickAndPlaceListener::Update, UpdateRate, true);
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

// Get grasped objects contact shape component
ISLContactShapeInterface* USLPickAndPlaceListener::GetContactShapeComponent(AActor* Actor) const
{
	if(Actor)
	{
		for (const auto& C : Actor->GetComponents())
		{
			if(C->Implements<USLContactShapeInterface>())
			{
				if(ISLContactShapeInterface* CSI = Cast<ISLContactShapeInterface>(C))
				{
					return CSI;
				}
			}
		}
	}
	return nullptr;
}

// Called when grasp starts
void USLPickAndPlaceListener::OnSLGraspBegin(const FSLEntity& Self, AActor* Other, float Time, const FString& GraspType)
{
	if(GraspedObject)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.. multiple objects grasped?"), *FString(__func__), __LINE__);
		return; // we are already grasping an object, ignore future ones
	}

	// Take into account only objects that have a contact shape component
	if(ISLContactShapeInterface* CSI = GetContactShapeComponent(Other))
	{
		GraspedObject = Other;
		GraspedObjectContactShape = CSI;

		PrevRelevantLocation = Other->GetActorLocation();
		PrevRelevantTime = GetWorld()->GetTimeSeconds();

		if(GraspedObjectContactShape->IsSupportedBySomething())
		{
			EventCheck = ESLPaPStateCheck::Slide;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen, should start with a supporting state.."),
				*FString(__func__), __LINE__);
			GraspedObject = nullptr;
			GraspedObjectContactShape = nullptr;
			return;
		}

		
		if(GetWorld()->GetTimerManager().IsTimerPaused(UpdateTimerHandle))
		{
			GetWorld()->GetTimerManager().UnPauseTimer(UpdateTimerHandle);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen, timer should have been paused.."),
				*FString(__func__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Grasped object %s does not have a ContactShapeInterface"),
			*FString(__func__), __LINE__, *Other->GetName());
		return; // we are already grasping an object, ignore future ones
	}
}

// Called when grasp ends
void USLPickAndPlaceListener::OnSLGraspEnd(const FSLEntity& Self, AActor* Other, float Time)
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.. multiple objects grasped?"),
				*FString(__func__), __LINE__);
	}
}

// Update callback
void USLPickAndPlaceListener::Update()
{
	if(GraspedObject == nullptr || GraspedObjectContactShape == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen....."), *FString(__func__), __LINE__);
		return;
	}

	if(EventCheck == ESLPaPStateCheck::Slide)
	{
		if(!GraspedObjectContactShape->IsSupportedBySomething())
		{
			const float CurrTime = GetWorld()->GetTimeSeconds();
			if(FVector::DistSquared(PrevRelevantLocation, GraspedObject->GetActorLocation()) > MinSlideDistSq
				&& CurrTime - PrevRelevantTime > MinSlideDuration)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d BCAST Slide [%f <--> %f]"),
					*FString(__func__), __LINE__, PrevRelevantTime, CurrTime);

				EventCheck = ESLPaPStateCheck::PickUpOrTransport;
			}
		}
	}
	else if(EventCheck == ESLPaPStateCheck::PickUpOrTransport)
	{
		if(!GraspedObjectContactShape->IsSupportedBySomething())
		{
			
		}
	}
}
