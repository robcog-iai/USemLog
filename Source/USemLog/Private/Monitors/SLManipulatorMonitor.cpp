// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLManipulatorMonitor.h"
#include "Monitors/SLManipulatorContactMonitorSphere.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"

#include "GameFramework/PlayerController.h"

#if SL_WITH_MC_GRASP
#include "MCGraspAnimController.h"
#endif // SL_WITH_MC_GRASP

// Sets default values for this component's properties
USLManipulatorMonitor::USLManipulatorMonitor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	bIgnore = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bGraspIsDirty = true;
	bIsPaused = false;
	InputAxisName = "LeftGrasp";
	bIsNotSkeletal = false;
	UnPauseTriggerVal = 0.5;
	
#if WITH_EDITOR
	// Default values
	HandType = ESLGraspHandType::Left;
#endif // WITH_EDITOR

	ActiveGraspType = "Default";
}

// Dtor
USLManipulatorMonitor::~USLManipulatorMonitor()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Init listener
bool USLManipulatorMonitor::Init(bool bInDetectGrasps, bool bInDetectContacts)
{
	if (bIgnore)
	{
		return false;
	}

	if (!bIsInit)
	{
		bDetectGrasps = bInDetectGrasps;
		bDetectContacts = bInDetectContacts;

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

		// Remove any unset references in the array
		Fingers.Remove(nullptr);

#if SL_WITH_MC_GRASP
		// Subscribe to grasp type changes
		 SubscribeToGraspTypeChanges();
#endif // SL_WITH_MC_GRASP
		

		// True if each group has at least one bone overlap
		if (LoadOverlapGroups())
		{
			for (auto BoneOverlap : GroupA)
			{
				BoneOverlap->Init(bDetectGrasps, bDetectContacts);
			}
			for (auto BoneOverlap : GroupB)
			{
				BoneOverlap->Init(bDetectGrasps, bDetectContacts);
			}

			bIsInit = true;
			return true;
		}
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void USLManipulatorMonitor::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Bind grasp trigger input and update check functions
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (UInputComponent* IC = PC->InputComponent)
			{
				IC->BindAxis(InputAxisName, this, &USLManipulatorMonitor::GraspInputAxisCallback);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d No Input Component found.."), *FString(__func__), __LINE__);
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No Player controller found.."), *FString(__func__), __LINE__);
			return;
		}

		// Start listening on the bone overlaps
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->Start();
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapContact);
				BoneOverlap->OnEndManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapGroupAGrasp);
				BoneOverlap->OnEndManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapGroupAGrasp);
			}
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Start();
			if (bDetectContacts)
			{
				BoneOverlap->OnBeginManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapContact);
				BoneOverlap->OnEndManipulatorContactOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapContact);
			}
			if (bDetectGrasps)
			{
				BoneOverlap->OnBeginManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnBeginOverlapGroupBGrasp);
				BoneOverlap->OnEndManipulatorGraspOverlap.AddUObject(this, &USLManipulatorMonitor::OnEndOverlapGroupBGrasp);
			}
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Pause/continue grasp detection
void USLManipulatorMonitor::PauseGraspDetection(bool bInPause)
{
	if (bInPause != bIsPaused)
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->PauseGrasp(bInPause);
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->PauseGrasp(bInPause);
		}
		bIsPaused = bInPause;


		// Clear sets
		if (bInPause)
		{
			for (const auto& Obj : GraspedObjects)
			{
				OnEndManipulatorGrasp.Broadcast(IndividualObject, Obj, GetWorld()->GetTimeSeconds());
			}
			GraspedObjects.Empty();
			SetA.Empty();
			SetB.Empty();
		}
	}
}

// Stop publishing grasp events
void USLManipulatorMonitor::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		for (auto BoneOverlap : GroupA)
		{
			BoneOverlap->Finish();
		}
		for (auto BoneOverlap : GroupB)
		{
			BoneOverlap->Finish();
		}
		
		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedGraspEvents)
		{
			OnEndManipulatorGrasp.Broadcast(IndividualObject, EvItr.OtherActor, EvItr.Time);
		}
		RecentlyEndedGraspEvents.Empty();

		// Publish dangling recently finished events
		for (const auto& EvItr : RecentlyEndedContactEvents)
		{
			OnEndManipulatorContact.Broadcast(IndividualObject, EvItr.Other, EvItr.Time);
		}
		RecentlyEndedContactEvents.Empty();

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLManipulatorMonitor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Set pre-defined parameters
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorMonitor, HandType))
	{
		if (HandType == ESLGraspHandType::Left)
		{
			InputAxisName = "LeftGrasp";
		}
		else if (HandType == ESLGraspHandType::Right)
		{
			InputAxisName = "RightGrasp";
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLManipulatorMonitor, bIsNotSkeletal))
	{
		Fingers.Empty();
	}
}
#endif // WITH_EDITOR

// Set overlap groups, return true if at least one valid overlap is in each group
bool USLManipulatorMonitor::LoadOverlapGroups()
{
	// Lambda to check grasp overlap components of owner and add them to their groups
	const auto GetOverlapComponentsLambda = [this](AActor* Owner)
	{
		TArray<UActorComponent*> GraspOverlaps = Owner->GetComponentsByClass(USLManipulatorContactMonitorSphere::StaticClass());
		for (UActorComponent* GraspOverlapComp : GraspOverlaps)
		{
			USLManipulatorContactMonitorSphere* GraspOverlap = CastChecked<USLManipulatorContactMonitorSphere>(GraspOverlapComp);
			if (GraspOverlap->GetGroup() == ESLManipulatorContactMonitorGroup::A)
			{
				GroupA.Add(GraspOverlap);
			}
			else if (GraspOverlap->GetGroup() == ESLManipulatorContactMonitorGroup::B)
			{
				GroupB.Add(GraspOverlap);
			}
		}
	};

	if (bIsNotSkeletal)
	{
		for (const auto& F : Fingers)
		{
			GetOverlapComponentsLambda(F);
		}
	}
	else 
	{
		GetOverlapComponentsLambda(GetOwner());
	}

	// Check if at least one valid overlap shape is in each group
	if (GroupA.Num() == 0 || GroupB.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d One of the grasp groups is empty grasp detection disabled."), *FString(__func__), __LINE__);
		bDetectGrasps = false;
	}
	return true;
}

/* Begin grasp related */
#if SL_WITH_MC_GRASP
// Subscribe to grasp type changes
bool USLManipulatorMonitor::SubscribeToGraspTypeChanges()
{
	if (UMCGraspAnimController* Sibling = CastChecked<UMCGraspAnimController>(
		GetOwner()->GetComponentByClass(UMCGraspAnimController::StaticClass())))
	{
		Sibling->OnGraspType.AddUObject(this, &USLManipulatorMonitor::OnGraspType);
		return true;
	}
	return false;
}

// Callback on grasp type change
void USLManipulatorMonitor::OnGraspType(const FString& Type)
{
	ActiveGraspType = Type;
	ActiveGraspType.RemoveFromStart("GA_");
	ActiveGraspType.RemoveFromEnd("_Left");
	ActiveGraspType.RemoveFromEnd("_Right");
	ActiveGraspType.Append("Grasp");
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d ActiveGraspType=%s"), *FString(__func__), __LINE__, *ActiveGraspType);
}
#endif // SL_WITH_MC_GRASP

// Check if the grasp trigger is active
void USLManipulatorMonitor::GraspInputAxisCallback(float Value)
{
	if (Value >= UnPauseTriggerVal)
	{
		PauseGraspDetection(false);
	}
	else
	{	
		PauseGraspDetection(true);
	}
}

// Process beginning of grasp in group A
void USLManipulatorMonitor::OnBeginOverlapGroupAGrasp(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetA.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupB.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}
}

// Process beginning of grasp in group B
void USLManipulatorMonitor::OnBeginOverlapGroupBGrasp(AActor* OtherActor)
{
	bool bAlreadyInSet = false;
	SetB.Emplace(OtherActor, &bAlreadyInSet);
	if (!bAlreadyInSet && GroupA.Num() != 0 && !GraspedObjects.Contains(OtherActor))
	{
		CheckGraspState();
	}
}

// Process ending of contact in group A
void USLManipulatorMonitor::OnEndOverlapGroupAGrasp(AActor* OtherActor)
{
	if (SetA.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* init from Group A. ~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			EndGrasp(OtherActor);
		}
	}
}

// Process ending of contact in group B
void USLManipulatorMonitor::OnEndOverlapGroupBGrasp(AActor* OtherActor)
{
	if (SetB.Remove(OtherActor) > 0)
	{
		if (GraspedObjects.Contains(OtherActor))
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* init from Group B. ~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
			EndGrasp(OtherActor);
		}
	}
}

// Check for grasping state
void USLManipulatorMonitor::CheckGraspState()
{
	for (const auto& Obj : SetA.Intersect(SetB))
	{
		if (!GraspedObjects.Contains(Obj))
		{
			BeginGrasp(Obj);
		}
	}
}

// A grasp has started
void USLManipulatorMonitor::BeginGrasp(AActor* OtherActor)
{
	// Check if it is a new grasp event, or a concatenation with a previous one, either way, there is a new grasp
	GraspedObjects.AddUnique(OtherActor);
	if(!SkipRecentGraspEndEventBroadcast(OtherActor, GetWorld()->GetTimeSeconds()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] \t ~~~~~~~~~~~~~~~~~~~~~~~~~~ *BEGIN GRASP BCAST* with %s ~~~~~~~~~~~~~~~~~~~~~~~~~~"),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), *OtherActor->GetName());
		OnBeginManipulatorGrasp.Broadcast(IndividualObject, OtherActor, GetWorld()->GetTimeSeconds(), ActiveGraspType);
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t\t ~~~~~~~~~~ *END GRASP* cancelled (concatenation) ~~~~~~~~~~"),
		//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
	}
}

// A grasp has ended
void USLManipulatorMonitor::EndGrasp(AActor* OtherActor)
{
	if (GraspedObjects.Remove(OtherActor) > 0)
	{
		// Grasp ended
		RecentlyEndedGraspEvents.Emplace(FSLGraspEndEvent(OtherActor, GetWorld()->GetTimeSeconds()));
		
		// Delay publishing for a while, in case the new event is of the same type and should be concatenated
		if(!GetWorld()->GetTimerManager().IsTimerActive(GraspDelayTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, &USLManipulatorMonitor::DelayedGraspEndEventCallback,
				MaxGraspEventTimeGap * 1.2f, false);
		}
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorMonitor::DelayedGraspEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > MaxGraspEventTimeGap)
		{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%f] \t ~~~~~~~~~~~~~~~~~~~~~~~~~~ *END GRASP BCAST* (with delay) GraspEnd=%f; with %s;  ~~~~~~~~~~~~~~~~~~~~~~~~~~"),
			//	*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(), EvItr->Time, *EvItr->OtherActor->GetName());
			// Broadcast delayed event
			OnEndManipulatorGrasp.Broadcast(IndividualObject, EvItr->OtherActor, EvItr->Time);
			
			// Remove event from the pending list
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedGraspEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(GraspDelayTimerHandle, this, &USLManipulatorMonitor::DelayedGraspEndEventCallback,
			MaxGraspEventTimeGap * 1.2f, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorMonitor::SkipRecentGraspEndEventBroadcast(AActor* OtherActor, float StartTime)
{
	for (auto EvItr(RecentlyEndedGraspEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->OtherActor == OtherActor)
		{
			// Check time difference
			if(StartTime - EvItr->Time < MaxGraspEventTimeGap)
			{
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedGraspEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(GraspDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
/* End grasp related */


/* Begin contact related */
// Process beginning of contact
void USLManipulatorMonitor::OnBeginOverlapContact(AActor* OtherActor)
{
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (int32* NumContacts = ObjectsInContact.Find(OtherActor))
	{
		(*NumContacts)++;
	}
	else
	{
		// Check if it is a new contact event, or a concatenation with a previous one, either way, there is a new contact
		ObjectsInContact.Add(OtherActor, 1);
		const float CurrTime = GetWorld()->GetTimeSeconds();
		if(!SkipRecentContactEndEventBroadcast(OtherIndividual, CurrTime))
		{
			// Broadcast begin of semantic overlap event
			OnBeginManipulatorContact.Broadcast(FSLContactResult(IndividualObject, OtherIndividual, CurrTime, false));
		}
	}

}

// Process ending of contact
void USLManipulatorMonitor::OnEndOverlapContact(AActor* OtherActor)
{
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (int32* NumContacts = ObjectsInContact.Find(OtherActor))
	{
		(*NumContacts)--;
			
		if ((*NumContacts) < 1)
		{
			// Remove contact object
			ObjectsInContact.Remove(OtherActor);
				
			if (!GetWorld())
			{
				// Episode already finished, continuing would be futile
				return;
			}

			// Manipulator contact ended
			RecentlyEndedContactEvents.Emplace(FSLContactEndEvent(OtherIndividual, GetWorld()->GetTimeSeconds()));
				
			// Delay publishing for a while, in case the new event is of the same type and should be concatenated
			if(!GetWorld()->GetTimerManager().IsTimerActive(ContactDelayTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this, &USLManipulatorMonitor::DelayedContactEndEventCallback,
					MaxContactEventTimeGap * 1.2f, false);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
	}

}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void USLManipulatorMonitor::DelayedContactEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = GetWorld()->GetTimeSeconds();
	
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// If enough time has passed, publish the event
		if(CurrTime - EvItr->Time > MaxContactEventTimeGap)
		{
			OnEndManipulatorContact.Broadcast(IndividualObject, EvItr->Other, EvItr->Time);
			EvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedContactEvents.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(ContactDelayTimerHandle, this, &USLManipulatorMonitor::DelayedContactEndEventCallback,
			MaxContactEventTimeGap * 1.2f, false);
	}
}

// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
bool USLManipulatorMonitor::SkipRecentContactEndEventBroadcast(USLBaseIndividual* InOther, float StartTime)
{
	for (auto EvItr(RecentlyEndedContactEvents.CreateIterator()); EvItr; ++EvItr)
	{
		// Check if it is an event between the same entities
		if(EvItr->Other == InOther)
		{
			// Check time difference between the previous and current event
			const float TimeGap = StartTime - EvItr->Time;
			if(TimeGap < MaxContactEventTimeGap)
			{
				// Event will be concatenated
				EvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedContactEvents.Num() == 0)
				{
					GetWorld()->GetTimerManager().ClearTimer(ContactDelayTimerHandle);
				}
				return true;
			}
		}
	}
	return false;
}
/* End contact related */
