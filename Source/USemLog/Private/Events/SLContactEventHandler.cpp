// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEventHandler.h"
#include "Monitors/SLContactMonitorInterface.h"
#include "Events/SLContactEvent.h"
#include "Events/SLSupportedByEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"


// Set parent
void FSLContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<ISLContactMonitorInterface>(InParent);
		if (Parent)
		{
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLContactEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (Parent)
		{
			Parent->OnBeginSLContact.AddRaw(this, &FSLContactEventHandler::OnSLOverlapBegin);
			Parent->OnEndSLContact.AddRaw(this, &FSLContactEventHandler::OnSLOverlapEnd);

			Parent->OnBeginSLSupportedBy.AddRaw(this, &FSLContactEventHandler::OnSLSupportedByBegin);
			Parent->OnEndSLSupportedBy.AddRaw(this, &FSLContactEventHandler::OnSLSupportedByEnd);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLContactEventHandler::Finish(float EndTime, bool bForced)
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
void FSLContactEventHandler::AddNewContactEvent(const FSLContactResult& InResult)
{
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> Event = MakeShareable(new FSLContactEvent(
		FSLUuid::NewGuidInBase64Url(), InResult.Time,
		FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
		InResult.Self, InResult.Other));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending contacts array
	StartedContactEvents.Emplace(Event);
}

// Publish finished event
bool FSLContactEventHandler::FinishContactEvent(USLBaseIndividual* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedContactEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			// Avoid publishing short events
			if (((*EventItr)->EndTime - (*EventItr)->StartTime) > ContactEventMin)
			{
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}
			
			// Remove event from the pending list
			EventItr.RemoveCurrent();

			return true;
		}
	}
	return false;
}

// Start new supported by event
void FSLContactEventHandler::AddNewSupportedByEvent(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 EventPairId)
{
	// Start a supported by event
	TSharedPtr<FSLSupportedByEvent> Event = MakeShareable(new FSLSupportedByEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EventPairId, Supported, Supporting));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending array
	StartedSupportedByEvents.Emplace(Event);
}

// Finish then publish the event
bool FSLContactEventHandler::FinishSupportedByEvent(const uint64 InPairId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedSupportedByEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->PairId == InPairId)
		{
			// Ignore short events
			if (EndTime - (*EventItr)->StartTime > SupportedByEventMin)
			{
				// Set end time and publish event
				(*EventItr)->EndTime = EndTime;
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLContactEventHandler::FinishAllEvents(float EndTime)
{
	// Finish contact events
	for (auto& Ev : StartedContactEvents)
	{
		// Ignore short events
		if (EndTime - Ev->StartTime > ContactEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedContactEvents.Empty();

	// Finish supported by events
	for (auto& Ev : StartedSupportedByEvents)
	{
		// Ignore short events
		if ((EndTime - Ev->StartTime) > SupportedByEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedSupportedByEvents.Empty();
}

// Event called when a semantic overlap event begins
void FSLContactEventHandler::OnSLOverlapBegin(const FSLContactResult& InResult)
{
	AddNewContactEvent(InResult);
}

// Event called when a semantic overlap event ends
void FSLContactEventHandler::OnSLOverlapEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishContactEvent(Other, Time);
}

// Event called when a supported by event begins
void FSLContactEventHandler::OnSLSupportedByBegin(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 PairId)
{
	AddNewSupportedByEvent(Supported, Supporting, StartTime, PairId);
}

// Event called when a 'possible' supported by event ends
void FSLContactEventHandler::OnSLSupportedByEnd(const uint64 PairId1, const uint64 PairId2, float EndTime)
{
	if(!FinishSupportedByEvent(PairId1, EndTime))
	{
		FinishSupportedByEvent(PairId2, EndTime);
	}
}
