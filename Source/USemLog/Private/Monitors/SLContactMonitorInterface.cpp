// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLContactMonitorInterface.h"
#include "Individuals/SLIndividualUtils.h"
#include "Components/MeshComponent.h"
#include "Utils/SLUuid.h"

// Stop publishing overlap events
void ISLContactMonitorInterface::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Publish any pending delayed events	
		for(const auto& Ev : RecentlyEndedOverlapEvents)
		{
			PublishDelayedOverlapEndEvent(Ev);
		}
		RecentlyEndedOverlapEvents.Empty();
		
		// Disable overlap events
		ShapeComponent->SetGenerateOverlapEvents(false);

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Init the interface
bool ISLContactMonitorInterface::InitContactMonitorInterface(UShapeComponent* InShapeComponent, UWorld* InWorld)
{
	if(InShapeComponent && InWorld)
	{
		World = InWorld;
		ShapeComponent = InShapeComponent;
		DelayTimerDelegate.BindRaw(this, &ISLContactMonitorInterface::DelayedOverlapEndEventCallback);
		return true;
	}
	return false;
}

// Publish currently overlapping components
void ISLContactMonitorInterface::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	ShapeComponent->GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		ISLContactMonitorInterface::OnOverlapBegin(
			ShapeComponent, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Start checking for supported by events
void ISLContactMonitorInterface::StartSupportedByUpdateCheck()
{
	if(World)
	{
		// Start updating the timer, will be paused if there are no candidates
		SupportedByTimerDelegate.BindRaw(this, &ISLContactMonitorInterface::SupportedByUpdateCheckBegin);
		World->GetTimerManager().SetTimer(SupportedByTimerHandle, SupportedByTimerDelegate, SupportedByUpdateRate, true);
	}
}

// Supported by update
// TODO is a supported by end update look required?
void ISLContactMonitorInterface::SupportedByUpdateCheckBegin()
{
	// Check if candidates are in a supported by event
	for (auto CandidateItr(SupportedByCandidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		// Get relative vertical speed
		const float RelVertSpeed = FMath::Abs(CandidateItr->SelfMeshComponent->GetComponentVelocity().Z -
			CandidateItr->OtherMeshComponent->GetComponentVelocity().Z);
		
		// Check that the relative speed on Z between the two objects is smaller than the threshold
		if (RelVertSpeed < SupportedByMaxVertSpeed)
		{
			if (CandidateItr->bIsOtherASemanticOverlapArea)
			{
				// Check which is supporting and which is supported
				// TODO simple height comparison for now
				if (CandidateItr->SelfMeshComponent->GetComponentLocation().Z >
					CandidateItr->OtherMeshComponent->GetComponentLocation().Z)
				{
					USLBaseIndividual* Supported = CandidateItr->Self;
					USLBaseIndividual* Supporting = CandidateItr->Other;
					const uint64 PairId = FSLUuid::PairEncodeCantor(Supported->GetUniqueID(), Supporting->GetUniqueID());
					OnBeginSLSupportedBy.Broadcast(Supported, Supporting, World->GetTimeSeconds(), PairId);
					IsSupportedByPariIds.Add(PairId);
				}
				else
				{
					USLBaseIndividual* Supported = CandidateItr->Other;
					USLBaseIndividual* Supporting = CandidateItr->Self;
					const uint64 PairId = FSLUuid::PairEncodeCantor(Supported->GetUniqueID(), Supporting->GetUniqueID());
					OnBeginSLSupportedBy.Broadcast(Supported, Supporting, World->GetTimeSeconds(), PairId);
					// Self item is supporting another, to not add it to the supportedby events id
				}
			}
			else 
			{
				// Other can only support, self can only be supported
				USLBaseIndividual* Supported = CandidateItr->Self;
				USLBaseIndividual* Supporting = CandidateItr->Other;
				const uint64 PairId = FSLUuid::PairEncodeCantor(Supported->GetUniqueID(), Supporting->GetUniqueID());
				OnBeginSLSupportedBy.Broadcast(Supported, Supporting, World->GetTimeSeconds(), PairId);
				IsSupportedByPariIds.Add(PairId);
			}
			// Remove candidate, it is now part of a started event
			CandidateItr.RemoveCurrent();
		}
	}
	
	// PauseGraspDetection timer
	if (SupportedByCandidates.Num() == 0)
	{
		World->GetTimerManager().PauseTimer(SupportedByTimerHandle);
	}
}

// Remove candidate from array
bool ISLContactMonitorInterface::CheckAndRemoveIfJustCandidate(USLBaseIndividual* InOther)
{
	// Use iterator to be able to remove the entry from the array
	for (auto CandidateItr(SupportedByCandidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		if ((*CandidateItr).Other == InOther)
		{
			// Remove candidate from the list
			CandidateItr.RemoveCurrent();
			return true; // Found
		}
	}
	return false; // Not in list
}

// Called on overlap begin events
void ISLContactMonitorInterface::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4fs \t BeginContact: \t %s:%s;"),
	//	*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *OtherActor->GetName(), *OtherComp->GetName());

	// Ignore self overlaps (area with static mesh)
	if (OtherActor == ShapeComponent->GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *OtherActor->GetName());
		return;
	}

	// Get the time of the event in second
	float StartTime = World->GetTimeSeconds();

	// Check if this overlap happened very closely to another finished one, if yes concatenate the two by ignoring this start
	// and the recent overlap end
	if(SkipOverlapEndEventBroadcast(OtherIndividual, StartTime))
	{
		return;
	}

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Broadcast begin of semantic overlap event
		FSLContactResult SemanticOverlapResult(OwnerIndividualObject, OtherIndividual,
			StartTime, false, OwnerMeshComp, OtherAsMeshComp);
		OnBeginSLContact.Broadcast(SemanticOverlapResult);

		if(bLogSupportedByEvents)
		{
			// Add candidate and re-start (if paused) timer cb
			SupportedByCandidates.Emplace(SemanticOverlapResult);
			if(World->GetTimerManager().IsTimerPaused(SupportedByTimerHandle))
			{
				World->GetTimerManager().UnPauseTimer(SupportedByTimerHandle);
			}
		}
	}
	else if (ISLContactMonitorInterface* OtherContactTrigger = Cast<ISLContactMonitorInterface>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherIndividual->GetUniqueID() > OwnerIndividualObject->GetUniqueID())
		{
			// Broadcast begin of semantic overlap event
			FSLContactResult SemanticOverlapResult(OwnerIndividualObject, OtherIndividual,
				StartTime, true, OwnerMeshComp, OtherContactTrigger->OwnerMeshComp);
			OnBeginSLContact.Broadcast(SemanticOverlapResult);
			
			if(bLogSupportedByEvents)
			{
				// Add candidate and re-start (if paused) timer cb
				SupportedByCandidates.Emplace(SemanticOverlapResult);
				if(World->GetTimerManager().IsTimerPaused(SupportedByTimerHandle))
				{
					World->GetTimerManager().UnPauseTimer(SupportedByTimerHandle);
				}
			}
		}
	}
}

// Called on overlap end events
void ISLContactMonitorInterface::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	//UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4fs \t EndContact: \t %s::%s;"),
	//	*FString(__FUNCTION__), __LINE__, World->GetTimeSeconds(), *OtherActor->GetName(), *OtherComp->GetName());

	// Ignore self overlaps (area with static mesh)
	if (OtherActor == ShapeComponent->GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor);
	if (OtherIndividual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Delay publishing the overlap event in case of possible concatenations
	RecentlyEndedOverlapEvents.Emplace(FSLOverlapEndEvent(OtherComp, OtherIndividual, World->GetTimeSeconds()));

	// Delay publishing for a while, in case the new event is of the same type and should be concatenated
	if(!World->GetTimerManager().IsTimerActive(DelayTimerHandle))
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		World->GetTimerManager().SetTimer(DelayTimerHandle, DelayTimerDelegate, DelayValue, false);
	}
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void ISLContactMonitorInterface::DelayedOverlapEndEventCallback()
{
	// Curr time (keep very recently added events for another delay)
	const float CurrTime = World->GetTimeSeconds();
	
	for (auto OverlapEndEvItr(RecentlyEndedOverlapEvents.CreateIterator()); OverlapEndEvItr; ++OverlapEndEvItr)
	{
		if(PublishDelayedOverlapEndEvent(*OverlapEndEvItr, CurrTime))
		{
			// Remove event from the pending list
			OverlapEndEvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyEndedOverlapEvents.Num() > 0)
	{
		const float DelayValue = ConcatenateIfSmaller + ConcatenateIfSmallerDelay;
		World->GetTimerManager().SetTimer(DelayTimerHandle, DelayTimerDelegate, DelayValue, false);
	}
}

// Broadcast delayed overlaps, if curr time < 0, it guarantees a publish
bool ISLContactMonitorInterface::PublishDelayedOverlapEndEvent(const FSLOverlapEndEvent& Ev, float CurrTime)
{
	// Check if the event is old enough that it had it chance to be concatenated
	// if CurrTime < 0, it forces publishing
	if(CurrTime < 0 || (CurrTime - Ev.Time > ConcatenateIfSmaller))
	{
		// Check the type of the other component
		if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(Ev.OtherComp))
		{
			// Broadcast end of semantic overlap event
			OnEndSLContact.Broadcast(OwnerIndividualObject, Ev.OtherIndividual, Ev.Time);
		}
		else if (ISLContactMonitorInterface* OtherContactTrigger = Cast<ISLContactMonitorInterface>(Ev.OtherComp))
		{
			// If both areas are trigger areas, they will both concurrently trigger overlap events.
			// To avoid this we consistently ignore one trigger event. This is chosen using
			// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
			// and consistently pick the event with a given (larger or smaller) value.
			// This allows us to be in sync with the overlap end event 
			// since the unique ids and the rule of ignoring the one event will not change
			// Filter out one of the trigger areas (compare unique ids)
			if (Ev.OtherIndividual->GetUniqueID() > OwnerIndividualObject->GetUniqueID())
			{
				// Broadcast end of semantic overlap event
				OnEndSLContact.Broadcast(OwnerIndividualObject, Ev.OtherIndividual, Ev.Time);
			}
		}

		if(bLogSupportedByEvents)
		{
			// Ignore and remove if it is a candidate only
			// (it cannot be a candidate and an event, e.g. contact ended with a candidate only)
			if(!CheckAndRemoveIfJustCandidate(Ev.OtherIndividual))
			{
				const uint64 PairId1 = FSLUuid::PairEncodeCantor(OwnerIndividualObject->GetUniqueID(), Ev.OtherIndividual->GetUniqueID());
				const uint64 PairId2 = FSLUuid::PairEncodeCantor(Ev.OtherIndividual->GetUniqueID(), OwnerIndividualObject->GetUniqueID());
				OnEndSLSupportedBy.Broadcast(PairId1, PairId2, Ev.Time);
				PrevSupportedByEndTime =  Ev.Time;
				if(IsSupportedByPariIds.Remove(PairId1) == 0)
				{
					IsSupportedByPariIds.Remove(PairId2);
				}
			}
		}
		return true;
	}
	return false;
}

// Skip publishing overlap event if it can be concatenated with the current event start
bool ISLContactMonitorInterface::SkipOverlapEndEventBroadcast(USLBaseIndividual* InIndividual, float StartTime)
{
	for (auto OverlapEndEvItr(RecentlyEndedOverlapEvents.CreateIterator()); OverlapEndEvItr; ++OverlapEndEvItr)
	{
		// Check if it is an event between the same entities
		if(OverlapEndEvItr->OtherIndividual == InIndividual)
		{
			// Check time difference
			if(StartTime - OverlapEndEvItr->Time < ConcatenateIfSmaller)
			{
				OverlapEndEvItr.RemoveCurrent();

				// Check if it was the last event, if so, pause the delay publisher
				if(RecentlyEndedOverlapEvents.Num() == 0)
				{
					World->GetTimerManager().ClearTimer(DelayTimerHandle);
				}
				
				return true;
			}
		}
	}
	return false;
}
