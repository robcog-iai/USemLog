// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEventHandler.h"
#include "SLContactShapeInterface.h"

// UUtils
#include "Ids.h"

// Set parent
void FSLContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<ISLContactShapeInterface>(InParent);
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
		// End and broadcast all started events
		FSLContactEventHandler::FinishAllEvents(EndTime);

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
	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
		FIds::NewGuidInBase64Url(), InResult.Time,
		FIds::PairEncodeCantor(InResult.Self.Obj->GetUniqueID(), InResult.Other.Obj->GetUniqueID()),
		InResult.Self, InResult.Other));
	// Add event to the pending contacts array
	StartedContactEvents.Emplace(ContactEvent);
}

// Publish finished event
bool FSLContactEventHandler::FinishContactEvent(UObject* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedContactEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Item2.Obj == InOther)
		{
			// Ignore short events
			if ((EndTime - (*EventItr)->Start) > ContactEventMin)
			{
				// Set end time and publish event
				(*EventItr)->End = EndTime;
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
void FSLContactEventHandler::AddNewSupportedByEvent(const FSLEntity& Supported, const FSLEntity& Supporting, float StartTime)
{
	// Start a supported by event
	TSharedPtr<FSLSupportedByEvent> Event = MakeShareable(new FSLSupportedByEvent(
		FIds::NewGuidInBase64Url(), StartTime,
		FIds::PairEncodeCantor(Supported.Obj->GetUniqueID(), Supporting.Obj->GetUniqueID()),
		Supported, Supporting));
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
			if ((EndTime - (*EventItr)->Start) > SupportedByEventMin)
			{
				// Set end time and publish event
				(*EventItr)->End = EndTime;
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
		if ((EndTime - Ev->Start) > ContactEventMin)
		{
			// Set end time and publish event
			Ev->End = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedContactEvents.Empty();

	// Finish supported by events
	for (auto& Ev : StartedSupportedByEvents)
	{
		// Ignore short events
		if ((EndTime - Ev->Start) > SupportedByEventMin)
		{
			// Set end time and publish event
			Ev->End = EndTime;
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
void FSLContactEventHandler::OnSLOverlapEnd(UObject* Self, UObject* Other, float Time)
{
	FinishContactEvent(Other, Time);
}

// Event called when a supported by event begins
void FSLContactEventHandler::OnSLSupportedByBegin(const FSLEntity& Supported, const FSLEntity& Supporting, float StartTime)
{
	AddNewSupportedByEvent(Supported, Supporting, StartTime);
}

// Event called when a 'possible' supported by event ends
void FSLContactEventHandler::OnSLSupportedByEnd(UObject* Supported, UObject* Supporting, float Time)
{
	const uint64 PairId = FIds::PairEncodeCantor(Supported->GetUniqueID(), Supporting->GetUniqueID());
	FinishSupportedByEvent(PairId, Time);
}
