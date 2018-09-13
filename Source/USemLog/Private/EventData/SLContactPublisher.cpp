// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLContactPublisher.h"
#include "SLOverlapArea.h"

// UUtils
#include "Ids.h"

// Constructor
FSLContactPublisher::FSLContactPublisher(USLOverlapArea* InParent)
{
	Parent = InParent;
}

// Init
void FSLContactPublisher::Init()
{
	Parent->OnBeginSLOverlap.AddRaw(this, &FSLContactPublisher::OnSLOverlapBegin);
	Parent->OnEndSLOverlap.AddRaw(this, &FSLContactPublisher::OnSLOverlapEnd);
}

// Terminate listener, finish and publish remaining events
void FSLContactPublisher::Finish(float EndTime)
{
	FSLContactPublisher::FinishAllEvents(EndTime);
}

// Start new contact event
void FSLContactPublisher::AddNewEvent(const FSLOverlapResult& InResult)
{
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
		FIds::NewGuidInBase64Url(),	InResult.TriggerTime, FIds::PairEncodeCantor(InResult.Id, Parent->OwnerId),
		Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass,
		InResult.Id, InResult.SemId, InResult.SemClass));
	// Add event to the pending contacts array
	StartedEvents.Emplace(ContactEvent);
}

// Publish finished event
bool FSLContactPublisher::FinishEvent(const uint32 InOtherId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Obj2Id == InOtherId)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			OnSemanticContactEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLContactPublisher::FinishAllEvents(float EndTime)
{
	// Finish contact events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticContactEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic overlap event begins
void FSLContactPublisher::OnSLOverlapBegin(const FSLOverlapResult& SemanticOverlapResult)
{
	FSLContactPublisher::AddNewEvent(SemanticOverlapResult);
}

// Event called when a semantic overlap event ends
void FSLContactPublisher::OnSLOverlapEnd(uint32 OtherId, float Time)
{
	FSLContactPublisher::FinishEvent(OtherId, Time);
}
