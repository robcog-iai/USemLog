// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLManipulatorContactEventHandler.h"
#include "Monitors/SLBoneContactMonitor.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Events/SLContactEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"

// Set parent
void FSLManipulatorContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLManipulatorMonitor>(InParent);
		if (Parent)
		{
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLManipulatorContactEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		Parent->OnBeginManipulatorContact.AddRaw(this, &FSLManipulatorContactEventHandler::OnSLOverlapBegin);
		Parent->OnEndManipulatorContact.AddRaw(this, &FSLManipulatorContactEventHandler::OnSLOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLManipulatorContactEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent first publish any pending (delayed) events
		if(!Parent->IsFinished())
		{
			Parent->Finish();
		}
		
		// End and broadcast all started events
		FinishAllEvents(EndTime);

		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Start new contact event
void FSLManipulatorContactEventHandler::AddNewEvent(const FSLContactResult& InResult)
{
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> Event = MakeShareable(new FSLContactEvent(
		FSLUuid::NewGuidInBase64Url(), InResult.Time,
		FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
		InResult.Self, InResult.Other));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending contacts array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLManipulatorContactEventHandler::FinishEvent(USLBaseIndividual* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			OnSemanticEvent.ExecuteIfBound(*EventItr);

			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLManipulatorContactEventHandler::FinishAllEvents(float EndTime)
{
	// Finish contact events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->EndTime = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic overlap event begins
void FSLManipulatorContactEventHandler::OnSLOverlapBegin(const FSLContactResult& SemanticOverlapResult)
{
	AddNewEvent(SemanticOverlapResult);
}

// Event called when a semantic overlap event ends
void FSLManipulatorContactEventHandler::OnSLOverlapEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishEvent(Other, Time);
}
