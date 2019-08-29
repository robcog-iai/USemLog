// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLManipulatorContactEventHandler.h"
#include "SLManipulatorOverlapSphere.h"
#include "SLManipulatorListener.h"

// UUtils
#include "Ids.h"

// Set parent
void FSLManipulatorContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLManipulatorListener>(InParent);
		if (Parent)
		{
			World = Parent->GetWorld();
			if(World)
			{
				DelayTimerDelegate.BindRaw(this, &FSLManipulatorContactEventHandler::DelayedFinishContactEvent);
				bIsInit = true;
			}
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
		// End and broadcast all started events
		FSLManipulatorContactEventHandler::FinishAllEvents(EndTime);

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
		// Check first if it should be concatenated to a recently finished event
	if(!ReOpenRecentlyFinishedContactEvent(InResult.Self, InResult.Other, InResult.Time))
	{
		// Start a semantic contact event
		TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
			FIds::NewGuidInBase64Url(), InResult.Time,
			FIds::PairEncodeCantor(InResult.Self.Obj->GetUniqueID(), InResult.Other.Obj->GetUniqueID()),
			InResult.Self, InResult.Other));
		// Add event to the pending contacts array
		StartedEvents.Emplace(ContactEvent);
	}
}

// Publish finished event
bool FSLManipulatorContactEventHandler::FinishEvent(const FSLEntity& InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Item2.EqualsFast(InOther))
		{
			// Set the event end time
			(*EventItr)->End = EndTime;

			// Add to the recently finished contact events
			RecentlyFinishedEvents.Emplace(*EventItr);
			
			// Remove event from the pending list
			EventItr.RemoveCurrent();

			// Delay publishing for a while, in case the new event is of the same type and should be concatenated
			if(!World->GetTimerManager().IsTimerActive(DelayTimerHandle))
			{
				World->GetTimerManager().SetTimer(DelayTimerHandle, DelayTimerDelegate,ContactEventTimeGapMax*2.f, false);
			}
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
		// Ignore short events
		if ((EndTime - Ev->Start) > ContactEventMin)
		{
			// Set end time and publish event
			Ev->End = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedEvents.Empty();

	// Finish the recently finished events
	for (auto& Ev : RecentlyFinishedEvents)
	{
		// Avoid publishing short events
		if ((Ev->End - Ev->Start) > ContactEventMin)
		{
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	RecentlyFinishedEvents.Empty();
}


// Event called when a semantic overlap event begins
void FSLManipulatorContactEventHandler::OnSLOverlapBegin(const FSLContactResult& SemanticOverlapResult)
{
	FSLManipulatorContactEventHandler::AddNewEvent(SemanticOverlapResult);
}

// Event called when a semantic overlap event ends
void FSLManipulatorContactEventHandler::OnSLOverlapEnd(const FSLEntity& Self, const FSLEntity& Other, float Time)
{
	FSLManipulatorContactEventHandler::FinishEvent(Other, Time);
}

// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
void FSLManipulatorContactEventHandler::DelayedFinishContactEvent()
{
	// Curr time (keep very recenlty added events for another delay)
	const float CurrTime = World->GetTimeSeconds();
	
	for (auto RecentEvItr(RecentlyFinishedEvents.CreateIterator()); RecentEvItr; ++RecentEvItr)
	{
		// Check if the event is old enough that it had it chance to be concatenated
		if(CurrTime - (*RecentEvItr)->End > ContactEventTimeGapMax)
		{
			// Avoid publishing short events
			if (((*RecentEvItr)->End - (*RecentEvItr)->Start) > ContactEventMin)
			{
				OnSemanticEvent.ExecuteIfBound(*RecentEvItr);
			}
			// Remove event from the pending list
			RecentEvItr.RemoveCurrent();
		}
	}

	// There are very recent events still available, spin another delay callback to give them a chance to concatenate
	if(RecentlyFinishedEvents.Num() > 0)
	{
		World->GetTimerManager().SetTimer(DelayTimerHandle, DelayTimerDelegate,ContactEventTimeGapMax*2.f, false);
	}
}

// Check if the new event should be concatenated to an existing finished one
bool FSLManipulatorContactEventHandler::ReOpenRecentlyFinishedContactEvent(const FSLEntity& Item1, const FSLEntity& Item2, float StartTime)
{
	for (auto RecentEvItr(RecentlyFinishedEvents.CreateIterator()); RecentEvItr; ++RecentEvItr)
	{
		// Check if it is an event between the same entities
		if((*RecentEvItr)->Item1.Obj == Item1.Obj && 
			(*RecentEvItr)->Item2.Obj == Item2.Obj)
		{
			// Check time difference
			if(StartTime - (*RecentEvItr)->End < ContactEventTimeGapMax)
			{
				StartedEvents.Emplace(*RecentEvItr);
				RecentEvItr.RemoveCurrent();
				return true;
			}
		}
	}
	return false;
}
