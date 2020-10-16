// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLPickAndPlaceMonitor.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Animation/SkeletalMeshActor.h"

// Sets default values for this component's properties
USLPickAndPlaceMonitor::USLPickAndPlaceMonitor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	bIgnore = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	
	CurrGraspedObj = nullptr;
	EventCheckState = ESLPaPStateCheck::NONE;
	UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

	/* PickUp */
	bLiftOffHappened = false;

	/* PutDown */
	RecentMovementBuffer.Reserve(RecentMovementBufferSize);
}

// Dtor
USLPickAndPlaceMonitor::~USLPickAndPlaceMonitor()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called when the game starts
void USLPickAndPlaceMonitor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame, used for timeline visualizations, activated and deactivated on request
void USLPickAndPlaceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	(this->*UpdateFunctionPtr)();
}


// Init listener
bool USLPickAndPlaceMonitor::Init()
{
	if (bIgnore)
	{
		return false;
	}

	if (!bIsInit)
	{
		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			IndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!IndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return false;
		}

		// Set the individual object
		IndividualObject = IndividualComponent->GetIndividualObject();

		// Init state
		EventCheckState = ESLPaPStateCheck::NONE;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

		bIsInit = true;
		return true;
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLPickAndPlaceMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe for grasp notifications from sibling component
		if(SubscribeForGraspEvents())
		{
			// Set tick update rate
			if(UpdateRate > 0.f)
			{
				SetComponentTickInterval(UpdateRate);
			}

			// Mark as started
			bIsStarted = true;
		}
	}
}

// Stop publishing grasp events
void USLPickAndPlaceMonitor::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Finish any active event
		FinishActiveEvent(EndTime);

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Subscribe for grasp events from sibling component
bool USLPickAndPlaceMonitor::SubscribeForGraspEvents()
{
	if(USLManipulatorMonitor* Sibling = CastChecked<USLManipulatorMonitor>(
		GetOwner()->GetComponentByClass(USLManipulatorMonitor::StaticClass())))
	{
		Sibling->OnBeginManipulatorGrasp.AddUObject(this, &USLPickAndPlaceMonitor::OnSLGraspBegin);
		Sibling->OnEndManipulatorGrasp.AddUObject(this, &USLPickAndPlaceMonitor::OnSLGraspEnd);

		return true;
	}
	return false;
}

// Get grasped objects contact shape component
ISLContactMonitorInterface* USLPickAndPlaceMonitor::GetContactMonitorComponent(AActor* Actor) const
{
	if(Actor)
	{
		for (const auto& C : Actor->GetComponents())
		{
			if(C->Implements<USLContactMonitorInterface>())
			{
				if(ISLContactMonitorInterface* CSI = Cast<ISLContactMonitorInterface>(C))
				{
					return CSI;
				}
			}
		}
	}
	return nullptr;
}

// Called when grasp starts
void USLPickAndPlaceMonitor::OnSLGraspBegin(USLBaseIndividual* Self, AActor* Other, float Time, const FString& GraspType)
{
	if(CurrGraspedObj)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] Cannot set %s as grasped object.. manipulator is already grasping %s;"),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName(), *CurrGraspedObj->GetName());
		return;
	}

	// Take into account only objects that have a contact shape component
	if(ISLContactMonitorInterface* ContactMonitor = GetContactMonitorComponent(Other))
	{
		CurrGraspedObj = Other;
		GraspedObjectContactMonitor = ContactMonitor;

		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s set as grasped object.."),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());

		PrevRelevantLocation = Other->GetActorLocation();
		PrevRelevantTime = GetWorld()->GetTimeSeconds();

		if(GraspedObjectContactMonitor->IsSupportedBySomething())
		{
			EventCheckState = ESLPaPStateCheck::Slide;
			UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_Slide;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s's PickAndPlace CheckState=Slide;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f]  %s's grasped object (%s) should be in a SupportedBy state.. aborting interaction.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetName());

			CurrGraspedObj = nullptr;
			GraspedObjectContactMonitor = nullptr;
			EventCheckState = ESLPaPStateCheck::NONE;
			UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s's PickAndPlace CheckState=NONE;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());

			return;
		}

		if (!IsComponentTickEnabled())
		{
			SetComponentTickEnabled(true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen, tick should have been disabled here.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		}
	}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] %s does not have a ContactMonitorInterface required to query the SupportedBy state..  aborting interaction.."),
	//		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());
	//}
}

// Called when grasp ends
void USLPickAndPlaceMonitor::OnSLGraspEnd(USLBaseIndividual* Self, AActor* Other, float Time)
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
		GraspedObjectContactMonitor = nullptr;
		EventCheckState = ESLPaPStateCheck::NONE;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

		// Terminate active event
		FinishActiveEvent(Time);

		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] %s removed as grasped object.."),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *Other->GetName());

		if (IsComponentTickEnabled())
		{
			SetComponentTickEnabled(false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen, tick should be running here.."),
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
void USLPickAndPlaceMonitor::FinishActiveEvent(float CurrTime)
{
	if(EventCheckState == ESLPaPStateCheck::Slide)
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## SLIDE ##############  [%f <--> %f]"),
		//	*FString(__func__), __LINE__, CurrTime, PrevRelevantTime, CurrTime);
		OnManipulatorSlideEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, CurrTime);
	}
	else if(EventCheckState == ESLPaPStateCheck::PickUp)
	{
		if(bLiftOffHappened)
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## PICK UP ##############  [%f <--> %f]"),
			//	*FString(__func__), __LINE__, CurrTime, PrevRelevantTime, CurrTime);
			OnManipulatorSlideEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, CurrTime);
			bLiftOffHappened = false;
		}
	}
	else if(EventCheckState == ESLPaPStateCheck::TransportOrPutDown)
	{
		//TODO
	}

	CurrGraspedObj = nullptr;
	EventCheckState = ESLPaPStateCheck::NONE;
	UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s's PickAndPlace CheckState=NONE;"),
		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
}

// Backtrace and check if a put-down event happened
bool USLPickAndPlaceMonitor::HasPutDownEventHappened(const float CurrTime, const FVector& CurrObjLocation, uint32& OutPutDownEndIdx)
{
	OutPutDownEndIdx = RecentMovementBuffer.Num() - 1;
	while(OutPutDownEndIdx > 0 && CurrTime - RecentMovementBuffer[OutPutDownEndIdx].Key < PutDownMovementBacktrackDuration)
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t\t [%f] [%f/%f] MinPutDownHeight"),
		//	*FString(__func__), __LINE__,
		//	GetWorld()->GetTimeSeconds(), 
		//	RecentMovementBuffer[OutPutDownEndIdx].Key,
		//	RecentMovementBuffer[OutPutDownEndIdx].Value.Z - CurrObjLocation.Z,
		//	MinPutDownHeight);

		if(RecentMovementBuffer[OutPutDownEndIdx].Value.Z - CurrObjLocation.Z > MinPutDownHeight)
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f]  \t\t\t\t PUT DOWN HAPPENED"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			return true;
		}
		OutPutDownEndIdx--;
	}
	//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f]  \t\t\t\t PUT DOWN HAS NOT HAPPENED"),
	//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
	return false;
}

// Update callback
void USLPickAndPlaceMonitor::Update()
{
	// Call the state update function
	(this->*UpdateFunctionPtr)();
}

/* Update functions*/
// Default update function
void USLPickAndPlaceMonitor::Update_NONE()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.."), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
}

// Check for slide events
void USLPickAndPlaceMonitor::Update_Slide()
{
	if(CurrGraspedObj == nullptr || GraspedObjectContactMonitor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] This should not happen.."), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		return;
	}

	const FVector CurrObjLocation = CurrGraspedObj->GetActorLocation();
	const float CurrTime = GetWorld()->GetTimeSeconds();
	const float CurrDistXY = FVector::DistXY(PrevRelevantLocation, CurrObjLocation);

	// Sliding events can only end when the object is not supported by the surface anymore
	if(!GraspedObjectContactMonitor->IsSupportedBySomething())
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f]  \t\t **** END SupportedBy ****"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());

		// Check if enough distance and time has passed for a sliding event
		if(CurrDistXY > MinSlideDistXY && CurrTime - PrevRelevantTime > MinSlideDuration)
		{
			const float ExactSupportedByEndTime = GraspedObjectContactMonitor->GetLastSupportedByEndTime();

			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## SLIDE ##############  [%f <--> %f]"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, ExactSupportedByEndTime);

			// Broadcast event
			OnManipulatorSlideEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, ExactSupportedByEndTime);

			// Only update if they were part of the sliding event
			PrevRelevantTime = ExactSupportedByEndTime;
			PrevRelevantLocation = CurrObjLocation;
		}

		bLiftOffHappened = false;
		EventCheckState = ESLPaPStateCheck::PickUp;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_PickUp;
	}
}

// Check for pick-up events
void USLPickAndPlaceMonitor::Update_PickUp()
{
	const FVector CurrObjLocation = CurrGraspedObj->GetActorLocation();
	const float CurrTime = GetWorld()->GetTimeSeconds();

	if(!GraspedObjectContactMonitor->IsSupportedBySomething())
	{
		if(bLiftOffHappened)
		{
			if(CurrObjLocation.Z - LiftOffLocation.Z > MaxPickUpHeight ||
				FVector::DistXY(LiftOffLocation, CurrObjLocation) > MaxPickUpDistXY)
			{

				//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## PICK UP ##############  [%f <--> %f]"),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);
				// Broadcast event
				OnManipulatorPickUpEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, CurrTime);

				// Start checking for the next possible events
				bLiftOffHappened = false;
				PrevRelevantTime = CurrTime;
				PrevRelevantLocation = CurrObjLocation;
				EventCheckState = ESLPaPStateCheck::TransportOrPutDown;
				UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_TransportOrPutDown;
				UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s's PickAndPlace CheckState=TransportOrPutDown;"),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
			}
		}
		else if(CurrObjLocation.Z - PrevRelevantLocation.Z > MinPickUpHeight)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f]  \t **** LiftOFF **** \t\t\t\t\t\t\t\t LIFTOFF"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());

			// This is not going to be the start time of the PickUp event, we use the SupportedBy end time
			// we save the LiftOffLocation to check against the ending of the PickUp event by comparing distances against
			bLiftOffHappened = true;
			LiftOffLocation = CurrObjLocation;
		}
		else if(FVector::DistXY(LiftOffLocation, PrevRelevantLocation) > MaxPickUpDistXY)
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f]  \t **** Skip PickUp **** \t\t\t\t\t\t\t\t SKIP PICKUP"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			EventCheckState = ESLPaPStateCheck::TransportOrPutDown;
			UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_TransportOrPutDown;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d [%.4f] %s's PickAndPlace CheckState=TransportOrPutDown;"),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] \t\t **** START SupportedBy ****"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());

		if(bLiftOffHappened)
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## PICK UP ##############  [%f <--> %f]"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);
			OnManipulatorPickUpEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, CurrTime);
		}

		// Start checking for next event
		PrevRelevantTime = CurrTime;
		PrevRelevantLocation = CurrObjLocation;
		EventCheckState = ESLPaPStateCheck::Slide;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_Slide;
	}
}

// Check for put-down or transport events
void USLPickAndPlaceMonitor::Update_TransportOrPutDown()
{
	const float CurrTime = GetWorld()->GetTimeSeconds();
	const FVector CurrObjLocation = CurrGraspedObj->GetActorLocation();

	if(GraspedObjectContactMonitor->IsSupportedBySomething())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f]  \t\t **** START SupportedBy ****"), *FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());

		// Check for the PutDown movement start time
		uint32 PutDownEndIdx = 0;
		if(HasPutDownEventHappened(CurrTime, CurrObjLocation, PutDownEndIdx))
		{
			float PutDownStartTime = -1.f;
			while(PutDownEndIdx > 0)
			{
				// Check when the 
				if(RecentMovementBuffer[PutDownEndIdx].Value.Z - CurrObjLocation.Z > MaxPutDownHeight
					|| FVector::Distance(RecentMovementBuffer[PutDownEndIdx].Value, CurrObjLocation) > MaxPutDownDistXY)
				{
					PutDownStartTime = RecentMovementBuffer[PutDownEndIdx].Key;

					//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## TRANSPORT ##############  [%f <--> %f]"),
					//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, PutDownStartTime);
					OnManipulatorTransportEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, PutDownStartTime);


					//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## PUT DOWN ##############  [%f <--> %f]"),
					//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PutDownStartTime, CurrTime);
					OnManipulatorPutDownEvent.Broadcast(IndividualObject, CurrGraspedObj, PutDownStartTime, CurrTime);
					break;
				}
				PutDownEndIdx--;
			}

			// If the limits are not crossed in the buffer the oldest available time is used (TODO, or should we ignore the action?)
			if(PutDownStartTime < 0)
			{
				//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] The limits were not crossed in the available data in the buffer, the oldest available time is used"),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
				PutDownStartTime = RecentMovementBuffer[0].Key;

				//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## TRANSPORT ##############  [%f <--> %f]"),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, PutDownStartTime);
				OnManipulatorPutDownEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, PutDownStartTime);

				//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## PUT DOWN ##############  [%f <--> %f]"),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PutDownStartTime, CurrTime);
				OnManipulatorPutDownEvent.Broadcast(IndividualObject, CurrGraspedObj, PutDownStartTime, CurrTime);
			}
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## TRANSPORT ##############  [%f <--> %f]"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PrevRelevantTime, CurrTime);
			OnManipulatorTransportEvent.Broadcast(IndividualObject, CurrGraspedObj, PrevRelevantTime, CurrTime);
		}

		RecentMovementBuffer.Reset(RecentMovementBufferSize);

		PrevRelevantTime = CurrTime;
		PrevRelevantLocation = CurrObjLocation;
		EventCheckState = ESLPaPStateCheck::Slide;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_Slide;
	}
	else
	{
		// Cache recent movements 
		if(RecentMovementBuffer.Num() > 1)
		{
			RecentMovementBuffer.Push(TPair<float, FVector>(CurrTime, CurrObjLocation));

			// Remove values older than RecentMovementBufferDuration
			int32 Count = 0;
			while(RecentMovementBuffer.Last().Key - RecentMovementBuffer[Count].Key > RecentMovementBufferDuration)
			{
				Count++;
			}
			if(Count > 0)
			{
				RecentMovementBuffer.RemoveAt(0, Count, false);
			}
		}
		else
		{
			RecentMovementBuffer.Push(TPair<float, FVector>(CurrTime, CurrObjLocation));
		}
	}
}


