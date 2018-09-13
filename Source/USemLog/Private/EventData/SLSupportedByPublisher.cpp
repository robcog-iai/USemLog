// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLSupportedByPublisher.h"
#include "SLOverlapArea.h"

// UUtils
#include "Ids.h"

#define SL_SB_VERT_SPEED_TH 0.5f // Supported by event vertical speed threshold
#define SL_SB_UPDATE_RATE_CB 0.15f // Update rate for the timer callback

// Default constructor
FSLSupportedByPublisher::FSLSupportedByPublisher(USLOverlapArea* InParent)
{
	Parent = InParent;
}

// Init
void FSLSupportedByPublisher::Init()
{
	Parent->OnBeginSLOverlap.AddRaw(this, &FSLSupportedByPublisher::OnSLOverlapBegin);
	Parent->OnEndSLOverlap.AddRaw(this, &FSLSupportedByPublisher::OnSLOverlapEnd);

	// Start timer (will be directly paused if no candidates are available)
	TimerDelegate.BindRaw(this, &FSLSupportedByPublisher::InspectCandidatesCb);
	Parent->GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		TimerDelegate, SL_SB_UPDATE_RATE_CB, true);
}


// Terminate listener, finish and publish remaining events
void FSLSupportedByPublisher::Finish(float EndTime)
{
	FSLSupportedByPublisher::FinishAllEvents(EndTime);
	Parent->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

// Check if other obj is a supported by candidate
bool FSLSupportedByPublisher::IsACandidate(const uint32 InOtherId, bool bRemoveIfFound)
{
	// Use iterator to be able to remove the entry from the array
	for (auto CandidateItr(Candidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		if ((*CandidateItr).Id == InOtherId)
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
void FSLSupportedByPublisher::InspectCandidatesCb()
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
		const float RelVerticalSpeed = FMath::Abs(Parent->OwnerMeshComp->GetComponentVelocity().Z -
			CandidateItr->MeshComponent->GetComponentVelocity().Z);
		
		// Check that the relative speed on Z between the two objects is smaller than the threshold
		if (RelVerticalSpeed < SL_SB_VERT_SPEED_TH)
		{
			const FString SemId = FIds::NewGuidInBase64Url();
			const uint64 PairId = FIds::PairEncodeCantor(CandidateItr->Id, Parent->OwnerId);
			if (CandidateItr->bIsSemanticOverlapArea)
			{
				// Check which is supporting and which is supported
				// TODO simple height comparison for now
				if (Parent->OwnerMeshComp->GetComponentLocation().Z >
					CandidateItr->MeshComponent->GetComponentLocation().Z)
				{
					StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
						SemId, StartTime, PairId,
						Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass, // supported
						CandidateItr->Id, CandidateItr->SemId, CandidateItr->SemClass))); // supporting
				}
				else
				{
					StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
						SemId, StartTime, PairId,
						CandidateItr->Id, CandidateItr->SemId, CandidateItr->SemClass, // supported
						Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass))); // supporting
				}
			}
			else 
			{
				// Can only support
				StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
					SemId, StartTime, PairId,
					Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass, // supported
					CandidateItr->Id, CandidateItr->SemId, CandidateItr->SemClass))); // supporting

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
bool FSLSupportedByPublisher::IsPartOfASupportedByEvent(FSLOverlapResult& InCandidate,
	float Time, TSharedPtr<FSLSupportedByEvent> OutEvent)
{
	// Get relative vertical speed
	const float RelVerticalSpeed = FMath::Abs(Parent->OwnerMeshComp->GetComponentVelocity().Z -
		InCandidate.MeshComponent->GetComponentVelocity().Z);

	// Check that the relative speed on Z between the two objects is smaller than the threshold
	if (RelVerticalSpeed < SL_SB_VERT_SPEED_TH)
	{
		// Disable overlaps until next tick
		//if (Parent->OwnerStaticMeshComp->GetGenerateOverlapEvents())
		//{
			Parent->OwnerMeshComp->SetGenerateOverlapEvents(false);

			// Re-enable overlaps next tick
			TimerDelegateNextTick.BindLambda([this]
			{
				Parent->OwnerMeshComp->SetGenerateOverlapEvents(true);
			});
			Parent->GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
		//}

		FHitResult ParentMoveDownHit;
		Parent->OwnerMeshComp->MoveComponent(FVector(0.f, 0.f, 20.5f),
			Parent->OwnerMeshComp->GetComponentQuat(), true, &ParentMoveDownHit,
			EMoveComponentFlags::MOVECOMP_DisableBlockingOverlapDispatch, ETeleportType::TeleportPhysics);
		
		//UE_LOG(LogTemp, Warning, TEXT(">> %s::%d PARENT MOVE DOWN HIT=\n\t%s"),
		//	TEXT(__FUNCTION__), __LINE__, *ParentMoveDownHit.ToString());

		FHitResult ParentMoveBackHit;
		Parent->OwnerMeshComp->MoveComponent(FVector(0.f, 0.f, 10.5f),
			Parent->OwnerMeshComp->GetComponentQuat(), true, &ParentMoveBackHit,
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
bool FSLSupportedByPublisher::FinishEvent(const uint64 InPairId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// Compare pair ids
		if ((*EventItr)->PairId == InPairId)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			OnSupportedByEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending events (this usually is called at end play)
void FSLSupportedByPublisher::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSupportedByEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}

// Event called when a semantic overlap event begins
void FSLSupportedByPublisher::OnSLOverlapBegin(const FSLOverlapResult& InResult)
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
void FSLSupportedByPublisher::OnSLOverlapEnd(uint32 OtherId, float Time)
{
	// Remove from candidate list
	if (!IsACandidate(OtherId, true))
	{
		// If not in candidate list, check if it is a started event, and finish it
		const uint64 PairId = FIds::PairEncodeCantor(OtherId, Parent->OwnerId);
		FSLSupportedByPublisher::FinishEvent(PairId, Time);
	}
}
