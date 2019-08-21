// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactShapeInterface.h"
#include "SLEntitiesManager.h"
#include "Components/MeshComponent.h"

// UUtils
#include "Tags.h"
#include "Ids.h"

// Stop publishing overlap events
void ISLContactShapeInterface::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Disable overlap events
		ShapeComponent->SetGenerateOverlapEvents(false);

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Publish currently overlapping components
void ISLContactShapeInterface::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	ShapeComponent->GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		ISLContactShapeInterface::OnOverlapBegin(
			ShapeComponent, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Start checking for supported by events
void ISLContactShapeInterface::StartSupportedBy()
{
	if(World)
	{
		// Start updating the timer, will be paused if there are no candidates
		SBTimerDelegate.BindRaw(this, &ISLContactShapeInterface::SupportedByUpdate);
		World->GetTimerManager().SetTimer(SBTimerHandle, SBTimerDelegate, SBUpdateRate, true);
	}
}

// Supported by update
void ISLContactShapeInterface::SupportedByUpdate()
{
	// Check if candidates are in a supported by event
	for (auto CandidateItr(SBCandidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		// Get relative vertical speed
		const float RelVertSpeed = FMath::Abs(CandidateItr->SelfMeshComponent->GetComponentVelocity().Z -
			CandidateItr->OtherMeshComponent->GetComponentVelocity().Z);
		
		// Check that the relative speed on Z between the two objects is smaller than the threshold
		if (RelVertSpeed < SBMaxVertSpeed)
		{
			if (CandidateItr->bIsOtherASemanticOverlapArea)
			{
				// Check which is supporting and which is supported
				// TODO simple height comparison for now
				if (CandidateItr->SelfMeshComponent->GetComponentLocation().Z >
					CandidateItr->OtherMeshComponent->GetComponentLocation().Z)
				{
					OnBeginSLSupportedBy.Broadcast(
						CandidateItr->Self,		// supported
						CandidateItr->Other,	// supporting
						World->GetTimeSeconds());
					SupportedByObj = CandidateItr->Other.Obj;
				}
				else
				{
					OnBeginSLSupportedBy.Broadcast(
						CandidateItr->Other,	// supported
						CandidateItr->Self,		// supporting
						World->GetTimeSeconds());
				}
			}
			else 
			{
				// Other can only support, self can only be supported
				OnBeginSLSupportedBy.Broadcast(
						CandidateItr->Self,		// supported
						CandidateItr->Other,	// supporting
						World->GetTimeSeconds());
				SupportedByObj = CandidateItr->Other.Obj;
			}
			// Remove candidate, it is now part of a started event
			CandidateItr.RemoveCurrent();
		}
	}
	
	// Pause timer
	if (SBCandidates.Num() == 0)
	{
		World->GetTimerManager().PauseTimer(SBTimerHandle);
	}
	
}

bool ISLContactShapeInterface::RemoveIfSupportedByCandidate(UObject* InOther)
{
	// Use iterator to be able to remove the entry from the array
	for (auto CandidateItr(SBCandidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		if ((*CandidateItr).Other.Obj == InOther)
		{
			// Remove candidate from the list
			CandidateItr.RemoveCurrent();

			return true; // Found
		}
	}
	return false; // Not in list
}

// Called on overlap begin events
void ISLContactShapeInterface::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps (area with static mesh)
	if (OtherActor == ShapeComponent->GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp);
	if (!OtherItem.IsSet())
	{
		// Other not valid, check if its outer is semantically annotated
		OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp->GetOuter());
		if (!OtherItem.IsSet())
		{
			return;
		}
	}

	// Get the time of the event in second
	float StartTime = World->GetTimeSeconds();

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Broadcast begin of semantic overlap event
		FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
			StartTime, false, OwnerMeshComp, OtherAsMeshComp);
		OnBeginSLContact.Broadcast(SemanticOverlapResult);

		if(bLogSupportedByEvents)
		{
			// Add candidate and re-start (if paused) timer cb
			SBCandidates.Emplace(SemanticOverlapResult);
			if(World->GetTimerManager().IsTimerPaused(SBTimerHandle))
			{
				World->GetTimerManager().UnPauseTimer(SBTimerHandle);
			}
		}
	}
	else if (ISLContactShapeInterface* OtherContactTrigger = Cast<ISLContactShapeInterface>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherItem.Obj->GetUniqueID() > SemanticOwner.Obj->GetUniqueID())
		{
			// Broadcast begin of semantic overlap event
			FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
				StartTime, true, OwnerMeshComp, OtherContactTrigger->OwnerMeshComp);
			OnBeginSLContact.Broadcast(SemanticOverlapResult);

			if(bLogSupportedByEvents)
			{
				// Add candidate and re-start (if paused) timer cb
				SBCandidates.Emplace(SemanticOverlapResult);
				if(World->GetTimerManager().IsTimerPaused(SBTimerHandle))
				{
					World->GetTimerManager().UnPauseTimer(SBTimerHandle);
				}
			}
		}
	}
}

// Called on overlap end events
void ISLContactShapeInterface::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps (area with static mesh)
	if (OtherActor == ShapeComponent->GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp);
	if (!OtherItem.IsSet())
	{
		// Other not valid, check if its outer is semantically annotated
		OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp->GetOuter());
		if (!OtherItem.IsSet())
		{
			return;
		}
	}

	// Get the time of the event in second
	float EndTime = World->GetTimeSeconds();

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Broadcast end of semantic overlap event
		OnEndSLContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);

		if(bLogSupportedByEvents)
		{
			// If the Other is not a supported by candidate, it could be part of e started supported by event
			if(!RemoveIfSupportedByCandidate(OtherItem.Obj))
			{
				OnEndSLSupportedBy.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);
			}
		}
	}
	else if (ISLContactShapeInterface* OtherContactTrigger = Cast<ISLContactShapeInterface>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherItem.Obj->GetUniqueID() > SemanticOwner.Obj->GetUniqueID())
		{
			// Broadcast end of semantic overlap event
			OnEndSLContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);

			if(bLogSupportedByEvents)
			{
				// If the Other is not a supported by candidate, it could be part of e started supported by event
				if(!RemoveIfSupportedByCandidate(OtherItem.Obj))
				{
					OnEndSLSupportedBy.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);
				}
			}
		}
	}
}