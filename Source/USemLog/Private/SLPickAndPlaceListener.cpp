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

	/* PickUp */
	bool bLiftOffHappened = false;
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

		// Terminate active event
		FinishActiveEvent();

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

// Object released, terminate active even
void USLPickAndPlaceListener::FinishActiveEvent()
{
	const float CurrTime = GetWorld()->GetTimeSeconds();

	if(EventCheck == ESLPaPStateCheck::Slide)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] BCAST Slide [%f <--> %f]"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);
		OnManipulatorSlideEvent.Broadcast(SemanticOwner, CurrGraspedObj, PrevRelevantTime, CurrTime);
	}
	else if(EventCheck == ESLPaPStateCheck::PickUpOrTransport)
	{
		// TODO add a ESLPaPStateCheck::PickUp?
		if(bLiftOffHappened)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] BCAST PickUp [%f <--> %f]"),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);
			OnManipulatorSlideEvent.Broadcast(SemanticOwner, CurrGraspedObj, PrevRelevantTime, CurrTime);
		}
	}
}

// Update callback
void USLPickAndPlaceListener::Update()
{
	// Call the state update function
	(this->*UpdateFunctionPtr)();



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
	UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.."), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
}

/* Update functions*/
// Check for slide events
void USLPickAndPlaceListener::Update_Slide()
{
	if(CurrGraspedObj == nullptr || GraspedObjectContactShape == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.."), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		return;
	}

	const FVector CurrObjLocation = CurrGraspedObj->GetActorLocation();
	const float CurrTime = GetWorld()->GetTimeSeconds();
	const float CurrDistXY = FVector::DistXY(PrevRelevantLocation, CurrObjLocation);

	// Sliding events can only end when the object is not supported by the surface anymore
	if(!GraspedObjectContactShape->IsSupportedBySomething())
	{
		// Check if enough distance and time has passed for a sliding event
		if(CurrDistXY > MinSlideDistXY && CurrTime - PrevRelevantTime > MinSlideDuration)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] BCAST Slide..  [%f <--> %f]"),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);

			// Broadcast event
			OnManipulatorSlideEvent.Broadcast(SemanticOwner, CurrGraspedObj, PrevRelevantTime, CurrTime);

			// Only update if they were part of the sliding event
			PrevRelevantTime = CurrTime;
			PrevRelevantLocation = CurrObjLocation;
		}

		EventCheck = ESLPaPStateCheck::PickUpOrTransport;
		UpdateFunctionPtr = &USLPickAndPlaceListener::Update_PickUpOrTransport;
	}
	//else
	//{
	//	if(CurrDistXY > MinSlideDistXY && CurrTime - PrevRelevantTime > MinSlideDuration)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Sliding.. DistXY=%f/%f; Duration=%f/%f; \t\t READY FOR BROADCAST DONE!"),
	//			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
	//			CurrDistXY, MinSlideDistXY,	CurrTime - PrevRelevantTime, MinSlideDuration);
	//	}
	//	else if(CurrDistXY > MinSlideDistXY)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Sliding.. DistXY=%f/%f; Duration=%f/%f; \t\t DIST DONE!"),
	//			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
	//			CurrDistXY, MinSlideDistXY,	CurrTime - PrevRelevantTime, MinSlideDuration);
	//	}
	//	else if(CurrTime - PrevRelevantTime > MinSlideDuration)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Sliding.. DistXY=%f/%f; Duration=%f/%f; \t\t DURATION DONE!"),
	//			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
	//			CurrDistXY, MinSlideDistXY,	CurrTime - PrevRelevantTime, MinSlideDuration);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Sliding.. DistXY=%f/%f; Duration=%f/%f;"),
	//			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
	//			CurrDistXY, MinSlideDistXY,	CurrTime - PrevRelevantTime, MinSlideDuration);
	//	}
	//}
}

// Check for pick-up or transport events
void USLPickAndPlaceListener::Update_PickUpOrTransport()
{
	const FVector CurrObjLocation = CurrGraspedObj->GetActorLocation();
	const float CurrTime = GetWorld()->GetTimeSeconds();

	if(!GraspedObjectContactShape->IsSupportedBySomething())
	{
		if(bLiftOffHappened)
		{
			const float CurrDistXY = FVector::DistXY(LiftOffLocation, CurrObjLocation);

			if(CurrDistXY > MaxPickUpDistXY || CurrObjLocation.Z - LiftOffLocation.Z > MaxPickUpHeight)
			{
				if(CurrDistXY > MaxPickUpDistXY)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] BCAST PickUp.. PickUpHeight=%f/%f; DistXY=%f/%f; [%f <--> %f] \t\t XY DISTANCE LIMIT REACHED"),
						*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
						CurrObjLocation.Z - LiftOffLocation.Z,
						MaxPickUpHeight,
						CurrDistXY,
						MaxPickUpDistXY,
						PrevRelevantTime,
						CurrTime);
				}
				else if(CurrObjLocation.Z - LiftOffLocation.Z > MaxPickUpHeight)
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] BCAST PickUp.. PickUpHeight=%f/%f; DistXY=%f/%f; [%f <--> %f] \t\t HEIGHT LIMIT REACHED"),
						*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
						CurrObjLocation.Z - LiftOffLocation.Z,
						MaxPickUpHeight,
						CurrDistXY,
						MaxPickUpDistXY,
						PrevRelevantTime,
						CurrTime);
				}

				// Broadcast event
				OnManipulatorPickUpEvent.Broadcast(SemanticOwner, CurrGraspedObj, PrevRelevantTime, CurrTime);

				// Reset relevant params
				PrevRelevantTime = CurrTime;
				PrevRelevantLocation = CurrObjLocation;

				// Start checking for the next possible events
				EventCheck = ESLPaPStateCheck::TransportOrPutDown;
				UpdateFunctionPtr = &USLPickAndPlaceListener::Update_TransportOrPutDown;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f]  PickUp Height=%f/%f; DistXY=%f/%f;"),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
					CurrObjLocation.Z - LiftOffLocation.Z,
					MaxPickUpHeight,
					CurrDistXY,
					MaxPickUpDistXY);
			}
		}
		else if(CurrObjLocation.Z - PrevRelevantLocation.Z > MinPickUpHeight)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] LiftOff happened"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());

			// This is not going to be the start time of the PickUp event, we use the SupportedBy end time
			// we save the LiftOffLocation to check against the ending of the PickUp event by comparing distances against
			bLiftOffHappened = true;
			LiftOffLocation = CurrObjLocation;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f]  Obj SupportedBy, check for sliding events"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());

		if(bLiftOffHappened)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] BCAST PickUp [%f <--> %f]"),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);
			OnManipulatorPickUpEvent.Broadcast(SemanticOwner, CurrGraspedObj, PrevRelevantTime, CurrTime);
		}

		// Start checking for next event
		PrevRelevantTime = CurrTime;
		PrevRelevantLocation = CurrObjLocation;
		EventCheck = ESLPaPStateCheck::Slide;
		UpdateFunctionPtr = &USLPickAndPlaceListener::Update_Slide;
	}
}

void USLPickAndPlaceListener::Update_TransportOrPutDown()
{
}
