// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Publishers/SLGraspPublisher.h"
#include "SLGraspTrigger.h"

// UUtils
#include "Ids.h"

// Constructor
FSLGraspPublisher::FSLGraspPublisher(USLGraspTrigger* InParent)
{
	Parent = InParent;
}

// Init
void FSLGraspPublisher::Init()
{
	// Subscribe to the forwarded semantically annotated grasping broadcasts
	Parent->OnBeginSLGrasp.AddRaw(this, &FSLGraspPublisher::OnSLGraspBegin);
	Parent->OnEndSLGrasp.AddRaw(this, &FSLGraspPublisher::OnSLGraspEnd);
}

// Terminate listener, finish and publish remaining events
void FSLGraspPublisher::Finish(float EndTime)
{
	FSLGraspPublisher::FinishAllEvents(EndTime);
}

// Start new grasp event
void FSLGraspPublisher::AddNewEvent(const FSLGraspResult& InBeginResult)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \t\t\t ADD NEW SL GRASP"), TEXT(__FUNCTION__), __LINE__);
	// Start a semantic grasp event
	TSharedPtr<FSLGraspEvent> Event = MakeShareable(new FSLGraspEvent(
		FIds::NewGuidInBase64Url(), InBeginResult.TriggerTime, FIds::PairEncodeCantor(InBeginResult.Id, Parent->OwnerId),
		Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass,
		InBeginResult.Id, InBeginResult.SemId, InBeginResult.SemClass));
	// Add event to the pending array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLGraspPublisher::FinishEvent(const uint32 InOtherId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->OtherId == InOtherId)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \t\t\t FINISH SL GRASP"), TEXT(__FUNCTION__), __LINE__);
			OnSemanticGraspEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending events (this usually is called at end play)
void FSLGraspPublisher::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticGraspEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic grasp event begins
void FSLGraspPublisher::OnSLGraspBegin(const FSLGraspResult& GraspBeginResult)
{
	FSLGraspPublisher::AddNewEvent(GraspBeginResult);
}

// Event called when a semantic grasp event ends
void FSLGraspPublisher::OnSLGraspEnd(uint32 OtherId, float Time)
{
	FSLGraspPublisher::FinishEvent(OtherId, Time);
}
