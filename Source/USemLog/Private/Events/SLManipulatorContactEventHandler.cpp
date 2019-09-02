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
	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
		FIds::NewGuidInBase64Url(), InResult.Time,
		FIds::PairEncodeCantor(InResult.Self.Obj->GetUniqueID(), InResult.Other.Obj->GetUniqueID()),
		InResult.Self, InResult.Other));
	// Add event to the pending contacts array
	StartedEvents.Emplace(ContactEvent);
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

			OnSemanticEvent.ExecuteIfBound(*EventItr);
			UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t CONTACT EVENT [%s;%s] [%f<-->%f]"),
				*FString(__func__), __LINE__, *(*EventItr)->Item1.Obj->GetName(), *(*EventItr)->Item2.Obj->GetName(), (*EventItr)->Start, (*EventItr)->End);
			
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
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
		UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t [FINISH]CONTACT EVENT [%s;%s] [%f<-->%f]"),
			*FString(__func__), __LINE__, *Ev->Item1.Obj->GetName(), *Ev->Item2.Obj->GetName(), Ev->Start, Ev->End);
	}
	StartedEvents.Empty();
}


// Event called when a semantic overlap event begins
void FSLManipulatorContactEventHandler::OnSLOverlapBegin(const FSLContactResult& SemanticOverlapResult)
{
	AddNewEvent(SemanticOverlapResult);
}

// Event called when a semantic overlap event ends
void FSLManipulatorContactEventHandler::OnSLOverlapEnd(const FSLEntity& Self, const FSLEntity& Other, float Time)
{
	FinishEvent(Other, Time);
}
