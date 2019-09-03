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
	
	CurrGraspedObj = nullptr;
	EventCheck = ESLPaPStateCheck::NONE;
	UpdateFunctionPtr = &USLPickAndPlaceListener::Update_NONE;
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

		// Init state
		EventCheck = ESLPaPStateCheck::NONE;
		UpdateFunctionPtr = &USLPickAndPlaceListener::Update_NONE;

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
	if(CurrGraspedObj)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Cannot set %s as grasped object.. manipulator is already grasping %s;"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
		return;
	}

	// Take into account only objects that have a contact shape component
	if(ISLContactShapeInterface* CSI = GetContactShapeComponent(Other))
	{
		CurrGraspedObj = Other;
		GraspedObjectContactShape = CSI;

		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s set as grasped object.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());

		PrevRelevantLocation = Other->GetActorLocation();
		PrevRelevantTime = GetWorld()->GetTimeSeconds();

		if(GraspedObjectContactShape->IsSupportedBySomething())
		{
			EventCheck = ESLPaPStateCheck::Slide;
			UpdateFunctionPtr = &USLPickAndPlaceListener::Update_Slide;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s should be in a SupportedBy state.. aborting interaction.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());

			CurrGraspedObj = nullptr;
			GraspedObjectContactShape = nullptr;
			EventCheck = ESLPaPStateCheck::NONE;
			UpdateFunctionPtr = &USLPickAndPlaceListener::Update_NONE;
			return;
		}

		
		if(GetWorld()->GetTimerManager().IsTimerPaused(UpdateTimerHandle))
		{
			GetWorld()->GetTimerManager().UnPauseTimer(UpdateTimerHandle);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen, timer should have been paused here.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s does not have a ContactShapeInterface required to query the SupportedBy state..  aborting interaction.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
	}
}

// Called when grasp ends
void USLPickAndPlaceListener::OnSLGraspEnd(const FSLEntity& Self, AActor* Other, float Time)
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
		GraspedObjectContactShape = nullptr;
		EventCheck = ESLPaPStateCheck::NONE;
		UpdateFunctionPtr = &USLPickAndPlaceListener::Update_NONE;

		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s removed as grasped object.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());

		if(!GetWorld()->GetTimerManager().IsTimerPaused(UpdateTimerHandle))
		{
			GetWorld()->GetTimerManager().PauseTimer(UpdateTimerHandle);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen, timer should have been running here.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] End grasp with %s while %s is still grasped.. ignoring event.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
	}
}

// Update callback
void USLPickAndPlaceListener::Update()
{
	// Call the state update function
	(this->*UpdateFunctionPtr)();

	if(CurrGraspedObj == nullptr || GraspedObjectContactShape == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen....."), *FString(__func__), __LINE__);
		return;
	}

	if(EventCheck == ESLPaPStateCheck::Slide)
	{

	}
	else if(EventCheck == ESLPaPStateCheck::PickUpOrTransport)
	{
		if(!GraspedObjectContactShape->IsSupportedBySomething())
		{
			
		}
	}
}

// Default update function
void USLPickAndPlaceListener::Update_NONE()
{

}

void USLPickAndPlaceListener::Update_Slide()
{
	if(!GraspedObjectContactShape->IsSupportedBySomething())
	{
		const float CurrTime = GetWorld()->GetTimeSeconds();
		const float CurrDistXY = FVector::DistXY(PrevRelevantLocation, CurrGraspedObj->GetActorLocation());
		if(CurrDistXY > MinSlideDistXY	&& CurrTime - PrevRelevantTime > MinSlideDuration)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d BCAST Slide [%f <--> %f]"),
				*FString(__func__), __LINE__, PrevRelevantTime, CurrTime);

			EventCheck = ESLPaPStateCheck::PickUpOrTransport;
			UpdateFunctionPtr = &USLPickAndPlaceListener::Update_PickUpOrTransport;
		}
	}
}

void USLPickAndPlaceListener::Update_PickUpOrTransport()
{
}

void USLPickAndPlaceListener::Update_TransportOrPutDown()
{
}
