// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSupportedByEventHandler.h"
#include "SLContactBox.h"

// UUtils
#include "Ids.h"

#define SL_SB_VERT_SPEED_TH 0.5f // Supported by event vertical speed threshold
#define SL_SB_UPDATE_RATE_CB 0.15f // Update rate for the timer callback

// Set parent
void FSLSupportedByEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{		// Check if parent is of right type
		Parent = Cast<USLContactBox>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}


// Bind to input delegates
void FSLSupportedByEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		Parent->OnBeginSLContact.AddRaw(this, &FSLSupportedByEventHandler::OnSLOverlapBegin);
		Parent->OnEndSLContact.AddRaw(this, &FSLSupportedByEventHandler::OnSLOverlapEnd);

		// Start timer (will be directly paused if no candidates are available)
		TimerDelegate.BindRaw(this, &FSLSupportedByEventHandler::InspectCandidatesCb);
		Parent->GetWorld()->GetTimerManager().SetTimer(TimerHandle,
			TimerDelegate, SL_SB_UPDATE_RATE_CB, true);

		// Mark as started
		bIsStarted = true;
	}
}


// Terminate listener, finish and publish remaining events
void FSLSupportedByEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// End and broadcast all started events
		FSLSupportedByEventHandler::FinishAllEvents(EndTime);
		if (Parent->GetWorld())
		{
			Parent->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}
		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject		

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Check if other obj is a supported by candidate
bool FSLSupportedByEventHandler::IsACandidate(UObject* InOther, bool bRemoveIfFound)
{
	// Use iterator to be able to remove the entry from the array
	for (auto CandidateItr(Candidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		if ((*CandidateItr).Other.Obj == InOther)
		{
			if (bRemoveIfFound)
			{
				// Remove candidate from the list
				CandidateItr.RemoveCurrent();
			}
			return true; // Found
		}
	}
	return false; // Not in list
}

// Check candidates vertical relative speed
void FSLSupportedByEventHandler::InspectCandidatesCb()
{
	// Current start time
	float StartTime = Parent->GetWorld()->GetTimeSeconds();
	
	// Check if candidates are in a supported by event
	for (auto CandidateItr(Candidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		// TODO use smarter checks
		//// Check if the candidate is in a supported by event with the parent
		//TSharedPtr<FSLSupportedByEvent> Event;
		//if (IsPartOfASupportedByEvent(*CandidateItr, StartTime, Event))
		//{
		//	if (Event.IsValid())
		//	{
		//		// Add event to the list
		//		StartedEvents.Emplace(Event);
		//		
		//		// Remove candidate, it is now part of a started event
		//		CandidateItr.RemoveCurrent();
		//	}
		//}

		// Get relative vertical speed
		const float RelVerticalSpeed = FMath::Abs(CandidateItr->SelfMeshComponent->GetComponentVelocity().Z -
			CandidateItr->OtherMeshComponent->GetComponentVelocity().Z);
		
		// Check that the relative speed on Z between the two objects is smaller than the threshold
		if (RelVerticalSpeed < SL_SB_VERT_SPEED_TH)
		{
			const FString Id = FIds::NewGuidInBase64Url();
			const uint64 PairId = FIds::PairEncodeCantor(CandidateItr->Self.Obj->GetUniqueID(), CandidateItr->Other.Obj->GetUniqueID());
			if (CandidateItr->bIsOtherASemanticOverlapArea)
			{
				// Check which is supporting and which is supported
				// TODO simple height comparison for now
				if (CandidateItr->SelfMeshComponent->GetComponentLocation().Z >
					CandidateItr->OtherMeshComponent->GetComponentLocation().Z)
				{
					StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
						Id, StartTime, PairId,
						CandidateItr->Self,			// supported
						CandidateItr->Other)));		// supporting
				}
				else
				{
					StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
						Id, StartTime, PairId,
						CandidateItr->Other,	// supported
						CandidateItr->Self)));	// supporting
				}
			}
			else 
			{
				// Can only support
				StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
					Id, StartTime, PairId,
					CandidateItr->Self,			// supported
					CandidateItr->Other)));		// supporting

			}
			// Remove candidate, it is now part of a started event
			CandidateItr.RemoveCurrent();
		}
	}

	// Pause timer
	if (Candidates.Num() == 0)
	{
		Parent->GetWorld()->GetTimerManager().PauseTimer(TimerHandle);
	}
}

// Is a supported by event, returns nullptr if not an event
bool FSLSupportedByEventHandler::IsPartOfASupportedByEvent(FSLContactResult& InCandidate,
	float Time, TSharedPtr<FSLSupportedByEvent> OutEvent)
{
	// Get relative vertical speed
	const float RelVerticalSpeed = FMath::Abs(InCandidate.SelfMeshComponent->GetComponentVelocity().Z -
		InCandidate.OtherMeshComponent->GetComponentVelocity().Z);

	// Check that the relative speed on Z between the two objects is smaller than the threshold
	if (RelVerticalSpeed < SL_SB_VERT_SPEED_TH)
	{
		// Disable overlaps until next tick
		//if (Parent->OwnerStaticMeshComp->GetGenerateOverlapEvents())
		//{
			InCandidate.SelfMeshComponent->SetGenerateOverlapEvents(false);

			// Re-enable overlaps next tick
			TimerDelegateNextTick.BindLambda([InCandidate]
			{
				InCandidate.SelfMeshComponent->SetGenerateOverlapEvents(true);
			});
			Parent->GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
		//}

		FHitResult ParentMoveDownHit;
		InCandidate.SelfMeshComponent->MoveComponent(FVector(0.f, 0.f, 20.5f),
			InCandidate.SelfMeshComponent->GetComponentQuat(), true, &ParentMoveDownHit,
			EMoveComponentFlags::MOVECOMP_DisableBlockingOverlapDispatch, ETeleportType::TeleportPhysics);
		
		//UE_LOG(LogTemp, Warning, TEXT(">> %s::%d PARENT MOVE DOWN HIT=\n\t%s"),
		//	*FString(__func__), __LINE__, *ParentMoveDownHit.ToString());

		FHitResult ParentMoveBackHit;
		InCandidate.SelfMeshComponent->MoveComponent(FVector(0.f, 0.f, 10.5f),
			InCandidate.SelfMeshComponent->GetComponentQuat(), true, &ParentMoveBackHit,
			EMoveComponentFlags::MOVECOMP_DisableBlockingOverlapDispatch, ETeleportType::TeleportPhysics);

		OutEvent = MakeShareable(new FSLSupportedByEvent());

		// Re-enable overlaps on next tick

		// Move Other mesh downwards

		// Move Parent mesh downwards
		
		return true;
	}
	
	return false;
}

// Publish finished event
bool FSLSupportedByEventHandler::FinishEvent(const uint64 InPairId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// Compare pair ids
		if ((*EventItr)->PairId == InPairId)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			OnSemanticEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending events (this usually is called at end play)
void FSLSupportedByEventHandler::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}

// Event called when a semantic overlap event begins
void FSLSupportedByEventHandler::OnSLOverlapBegin(const FSLContactResult& InResult)
{
	// Add as candidate
	Candidates.Emplace(InResult);

	// Re-start (if paused) timer to check candidates
	if (Parent->GetWorld()->GetTimerManager().IsTimerPaused(TimerHandle))
	{
		Parent->GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle);
	}
}

// Event called when a semantic overlap event ends
void FSLSupportedByEventHandler::OnSLOverlapEnd(UObject* Self, UObject* Other, float Time)
{
	// Remove from candidate list
	if (!IsACandidate(Other, true))
	{
		// If not in candidate list, check if it is a started event, and finish it
		const uint64 PairId = FIds::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID());
		FSLSupportedByEventHandler::FinishEvent(PairId, Time);
	}
}
