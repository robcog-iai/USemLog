// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
	bLogDebug = false;
	bLogVerboseDebug = false;
	bLogAllEventsDebug = false;
	bLogSlideDebug = false;
	bLogPickUpDebug = false;
	bLogTransportPutDownDebug = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	
	// Default values
	UpdateRate = 0.029f;

	// Slide detection
	MinSlideDistXY = 9.f;
	MinSlideDuration = 0.9f;
	
	// PickUp detection
	MaxPickUpDistXY = 9.f;
	MinPickUpHeight = 3.f;
	MaxPickUpHeight = 12.f;

	// PutDown
	MinPutDownHeight = 2.f;
	MaxPutDownHeight = 8.f;
	MaxPutDownDistXY = 9.f;

	// Event check defaults
	CurrGraspedIndividual = nullptr;
	EventCheckState = ESLPaPStateCheck::NONE;
	UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

	/* PickUp */
	bPickUpHappened = false;

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
void USLPickAndPlaceMonitor::Init()
{
	if (bIgnore)
	{
		return;
	}

	if (!bIsInit)
	{
		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			OwnerIndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!OwnerIndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return;
			}

			// Set the individual object
			OwnerIndividualObject = OwnerIndividualComponent->GetIndividualObject();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return;
		}

		// Init state
		EventCheckState = ESLPaPStateCheck::NONE;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

		bIsInit = true;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully initialized %s::%s at %.4fs.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
	}
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
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully started %s::%s at %.4fs.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
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

		// Clear references
		CurrGraspedIndividual = nullptr;
		GraspedObjectContactMonitor = nullptr;
		EventCheckState = ESLPaPStateCheck::NONE;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

		// Make sure tick is disabled
		SetComponentTickEnabled(false);

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;

		// TODO - Commented out since GetWorld() can crash in newer versions in nullptr
		//if (GetWorld())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s at %.4fs.."),
		//		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName(), GetWorld()->GetTimeSeconds());
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Succefully finished %s::%s at unknown (world not valid).."),
		//		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *GetName());
		//}
	}
}

// Subscribe for grasp events from sibling component
bool USLPickAndPlaceMonitor::SubscribeForGraspEvents()
{
	if (auto* AC = GetOwner()->GetComponentByClass(USLManipulatorMonitor::StaticClass()))
	{
		USLManipulatorMonitor* ManipulatorMonitor = CastChecked<USLManipulatorMonitor>(AC);
		ManipulatorMonitor->OnBeginManipulatorGrasp.AddUObject(this, &USLPickAndPlaceMonitor::OnManipulatorGraspBegin);
		ManipulatorMonitor->OnEndManipulatorGrasp.AddUObject(this, &USLPickAndPlaceMonitor::OnManipulatorGraspEnd);
		return true;
	}
	return false;
}

// Called when grasp starts
void USLPickAndPlaceMonitor::OnManipulatorGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& GraspType)
{
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Grasp event started received: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	if(CurrGraspedIndividual)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasp individual is already set (%s), this can happen if grasping mulitple individuals, ignoring grasp %s.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
		return;
	}

	// Take into account only objects that have a contact shape component
	if(ISLContactMonitorInterface* ContactMonitor = GetContactMonitorComponent(Other->GetParentActor()))
	{
		// Set the grasped individual and its contact monitor
		CurrGraspedIndividual = Other;
		GraspedObjectContactMonitor = ContactMonitor;

		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual and contact monitor set, owner=%s.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*Other->GetParentActor()->GetName());
		}

		// Cache locations and time
		PrevRelevantLocation = Other->GetParentActor()->GetActorLocation();
		PrevRelevantTime = GetWorld()->GetTimeSeconds();

		// Make sure the grasped individual is in a supported by state
		if(GraspedObjectContactMonitor->IsSupportedBySomething())
		{
			if (bLogAllEventsDebug || bLogSlideDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual (%s) is in a supported by state, checking for slide events.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetParentActor()->GetName());
			}
			EventCheckState = ESLPaPStateCheck::Slide;
			UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_Slide;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasped individual (%s) is not in a supported by state.. aborting interaction.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetName());

			CurrGraspedIndividual = nullptr;
			GraspedObjectContactMonitor = nullptr;
			EventCheckState = ESLPaPStateCheck::NONE;
			UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;
			return;
		}

		// Start update
		if (!IsComponentTickEnabled())
		{
			SetComponentTickEnabled(true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's tick shoudl had been disabled, this should not happen.."),
				*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		}
	}
	else
	{
		if (bLogDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual (%s) does not have a contact monitor, ignoring.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
		}
	}
}

// Called when grasp ends
void USLPickAndPlaceMonitor::OnManipulatorGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	if (bLogDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's Grasp event ended received: \t %s->%s;"), *FString(__FUNCTION__), __LINE__,
			GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*Self->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	if (CurrGraspedIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's no individual is set as grasped (whilst grasp ended with %s), this should not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), *Other->GetParentActor()->GetName());
		return;
	}

	if (CurrGraspedIndividual != Other)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's unrelated grasp ended (%s != %s), this can happen if grasping multiple individuals.. skipping.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
		return;
	}

	if (bLogDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasp ended (%s==%s) finishing any active events.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName(), *Other->GetParentActor()->GetName());
	}

	// Grasp ended, terminate any current event
	FinishActiveEvent(Time);

	// Clear references
	CurrGraspedIndividual = nullptr;
	GraspedObjectContactMonitor = nullptr;
	EventCheckState = ESLPaPStateCheck::NONE;
	UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_NONE;

	// Stop update
	if (IsComponentTickEnabled())
	{
		SetComponentTickEnabled(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's tick should had been enabled, this should not happen.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}
}

// Object released, terminate active even
void USLPickAndPlaceMonitor::FinishActiveEvent(float CurrTime)
{
	if(EventCheckState == ESLPaPStateCheck::Slide)
	{
		if (bLogAllEventsDebug || bLogSlideDebug)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's active event ended **SLIDE** (%.4f-%.4f) event happened.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), PrevRelevantTime, CurrTime);
		}
		OnManipulatorSlideEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, CurrTime);
	}
	else if(EventCheckState == ESLPaPStateCheck::PickUp)
	{
		if(bPickUpHappened)
		{
			if (bLogAllEventsDebug || bLogPickUpDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's active event ended **PICK-UP** (%.4f-%.4f) event happened.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), PrevRelevantTime, CurrTime);
			}
			OnManipulatorPickUpEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, CurrTime);
			bPickUpHappened = false;
		}
	}
	else if(EventCheckState == ESLPaPStateCheck::TransportOrPutDown)
	{
		//TODO
	}
}

// Backtrace and check if a put-down event happened
bool USLPickAndPlaceMonitor::HasPutDownEventHappened(const float CurrTime, const FVector& CurrObjLocation, uint32& OutPutDownEndIdx)
{
	if (bLogAllEventsDebug || bLogTransportPutDownDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s checking if put-down happened.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}

	// Backtrack movement buffer and see when put-down might have started
	OutPutDownEndIdx = RecentMovementBuffer.Num() - 1;
	while(OutPutDownEndIdx > 0 && CurrTime - RecentMovementBuffer[OutPutDownEndIdx].Key < PutDownMovementBacktrackDuration)
	{
		if(RecentMovementBuffer[OutPutDownEndIdx].Value.Z - CurrObjLocation.Z > MinPutDownHeight)
		{
			if (bLogAllEventsDebug || bLogTransportPutDownDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s put-down happened at index=%d.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(), OutPutDownEndIdx);
			}
			return true;
		}
		OutPutDownEndIdx--;
	}

	// No put-down has been found in the movement buffer
	if (bLogAllEventsDebug || bLogTransportPutDownDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s put-down did not happen.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
	}
	return false;
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
	if(CurrGraspedIndividual == nullptr || GraspedObjectContactMonitor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's this should not happen, the grasped individual needs to be set during update check.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName());
		return;
	}

	const FVector CurrObjLocation = CurrGraspedIndividual->GetParentActor()->GetActorLocation();
	const float CurrTime = GetWorld()->GetTimeSeconds();
	const float CurrDistXY = FVector::DistXY(PrevRelevantLocation, CurrObjLocation);

	if (bLogVerboseDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's checking if %s is still supported by.."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName());
	}

	// Sliding events can only end when the object is not supported by the surface anymore
	if(!GraspedObjectContactMonitor->IsSupportedBySomething())
	{
		if (bLogAllEventsDebug || bLogSlideDebug || bLogPickUpDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s is not supported by anymore, check if slide happened, starting pick-up event check.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*CurrGraspedIndividual->GetParentActor()->GetName());
		}

		// Check if enough distance and time has passed for a sliding event
		if((CurrDistXY > MinSlideDistXY) && (CurrTime - PrevRelevantTime > MinSlideDuration))
		{
			const float ExactSupportedByEndTime = GraspedObjectContactMonitor->GetLastSupportedByEndTime();

			if (bLogAllEventsDebug || bLogSlideDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's supported by ended on %s **SLIDE** (%.4f-%.4f) event happened.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName(), PrevRelevantTime, ExactSupportedByEndTime);
			}

			// Broadcast event
			OnManipulatorSlideEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, ExactSupportedByEndTime);

			// Only update if they were part of the sliding event
			PrevRelevantTime = ExactSupportedByEndTime;
			PrevRelevantLocation = CurrObjLocation;
		}
		else
		{
			if (bLogAllEventsDebug || bLogSlideDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s is **NO** **SLIDE** happned.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName());
			}
		}

		// Set check for pick-up
		bPickUpHappened = false;
		EventCheckState = ESLPaPStateCheck::PickUp;
		UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_PickUp;
	}
}

// Check for pick-up events
void USLPickAndPlaceMonitor::Update_PickUp()
{
	const FVector CurrObjLocation = CurrGraspedIndividual->GetParentActor()->GetActorLocation();
	const float CurrTime = GetWorld()->GetTimeSeconds();

	if (bLogVerboseDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's pickup update checking on %s...."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName());
	}

	// Check distances whilst the object is not supported by
	if(!GraspedObjectContactMonitor->IsSupportedBySomething())
	{
		if(bPickUpHappened)
		{
			if(CurrObjLocation.Z - LiftOffLocation.Z > MaxPickUpHeight ||
				FVector::DistXY(LiftOffLocation, CurrObjLocation) > MaxPickUpDistXY)
			{
				if (bLogAllEventsDebug || bLogPickUpDebug)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s **PICK-UP** (%.4f-%.4f) event happened.."),
						*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
						*CurrGraspedIndividual->GetParentActor()->GetName(), PrevRelevantTime, CurrTime);
				}

				// Broadcast event
				OnManipulatorPickUpEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, CurrTime);

				if (bLogAllEventsDebug || bLogTransportPutDownDebug)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s was picked-up starting checking for transport+put-down event.."),
						*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
						*CurrGraspedIndividual->GetParentActor()->GetName());
				}

				// Start checking for the next possible events
				bPickUpHappened = false;
				PrevRelevantTime = CurrTime;
				PrevRelevantLocation = CurrObjLocation;
				EventCheckState = ESLPaPStateCheck::TransportOrPutDown;
				UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_TransportOrPutDown;
			}
		}
		else if(CurrObjLocation.Z - PrevRelevantLocation.Z > MinPickUpHeight)
		{
			if (bLogAllEventsDebug || bLogPickUpDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grapsed obj %s, setting pick-up flag to TRUE, event will be published next tick.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName());
			}

			// This is not going to be the start time of the PickUp event, we use the SupportedBy end time
			// we save the LiftOffLocation to check against the ending of the PickUp event by comparing distances against
			bPickUpHappened = true;
			LiftOffLocation = CurrObjLocation;
		}
		else if(FVector::DistXY(LiftOffLocation, PrevRelevantLocation) > MaxPickUpDistXY)
		{
			if (bLogAllEventsDebug || bLogPickUpDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grapsed obj %s, skipping pick-up, event did not happen.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName());
			}

			if (bLogAllEventsDebug || bLogTransportPutDownDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s was NOT picked up, starting checking for transport+put-down event.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName());
			}

			EventCheckState = ESLPaPStateCheck::TransportOrPutDown;
			UpdateFunctionPtr = &USLPickAndPlaceMonitor::Update_TransportOrPutDown;
		}
	}
	else
	{
		if (bLogAllEventsDebug || bLogSlideDebug || bLogPickUpDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s is supported by again, starting checking for slide event.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*CurrGraspedIndividual->GetParentActor()->GetName());
		}

		if(bPickUpHappened)
		{
			if (bLogAllEventsDebug || bLogPickUpDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s **PICK-UP** (%.4f-%.4f) event happened before supported by.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName(), PrevRelevantTime, CurrTime);
			}
			OnManipulatorPickUpEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, CurrTime);
		}
		else
		{
			if (bLogAllEventsDebug || bLogPickUpDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs %s's grasped individual %s **NO** **PICK-UP**.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
					*CurrGraspedIndividual->GetParentActor()->GetName());
			}
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
	const FVector CurrObjLocation = CurrGraspedIndividual->GetParentActor()->GetActorLocation();

	if (bLogVerboseDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's transport+put-down update checking on %s...."),
			*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
			*CurrGraspedIndividual->GetParentActor()->GetName());
	}

	// Check if the grasped object is supported by
	if(GraspedObjectContactMonitor->IsSupportedBySomething())
	{
		// No put-down has been found in the movement buffer
		if (bLogAllEventsDebug || bLogTransportPutDownDebug)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasped object %s is started being supported by.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
				*GetOwner()->GetName(), *CurrGraspedIndividual->GetParentActor()->GetName());
		}

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

					if (bLogAllEventsDebug || bLogTransportPutDownDebug)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasped object %s **TRASNPORT** (%.4f-%.4f) with **PUT-DOWN** (%.4f-%.4f) .."),
							*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
							*GetOwner()->GetName(), *CurrGraspedIndividual->GetParentActor()->GetName(),
							PrevRelevantTime, PutDownStartTime, PutDownStartTime, CurrTime);
					}
					OnManipulatorTransportEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, PutDownStartTime);
					OnManipulatorPutDownEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PutDownStartTime, CurrTime);
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
				OnManipulatorPutDownEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, PutDownStartTime);

				//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ############## PUT DOWN ##############  [%f <--> %f]"),
				//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), PutDownStartTime, CurrTime);
				OnManipulatorPutDownEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PutDownStartTime, CurrTime);
			}
		}
		else
		{
			if (bLogAllEventsDebug || bLogTransportPutDownDebug)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasped object %s **TRASNPORT** (%.4f-%.4f) without **PUT-DOWN**.."),
					*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(),
					*GetOwner()->GetName(), *CurrGraspedIndividual->GetParentActor()->GetName(),
					PrevRelevantTime, CurrTime);
			}
			OnManipulatorTransportEvent.Broadcast(OwnerIndividualObject, CurrGraspedIndividual, PrevRelevantTime, CurrTime);
		}

		// Clear movement buffer
		RecentMovementBuffer.Reset(RecentMovementBufferSize);

		if (bLogAllEventsDebug || bLogSlideDebug)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs %s's grasped individual %s is supported by again, starting checking for slide event.."),
				*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds(), *GetOwner()->GetName(),
				*CurrGraspedIndividual->GetParentActor()->GetName());
		}

		// Start checking for slide events until the grasped individual is released
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

// Get grasped objects contact shape component
ISLContactMonitorInterface* USLPickAndPlaceMonitor::GetContactMonitorComponent(AActor* Actor) const
{
	for (const auto& C : Actor->GetComponents())
	{
		if (C->Implements<USLContactMonitorInterface>())
		{
			if (ISLContactMonitorInterface* CSI = Cast<ISLContactMonitorInterface>(C))
			{
				return CSI;
			}
		}
	}
	return nullptr;
}

