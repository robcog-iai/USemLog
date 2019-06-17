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
			// Mark as initialized
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
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
		FIds::NewGuidInBase64Url(), InResult.Time,
		FIds::PairEncodeCantor(InResult.Self.Obj->GetUniqueID(), InResult.Other.Obj->GetUniqueID()),
		InResult.Self, InResult.Other));
	// Add event to the pending contacts array
	StartedEvents.Emplace(ContactEvent);
}

// Publish finished event
bool FSLManipulatorContactEventHandler::FinishEvent(UObject* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Item2.Obj == InOther)
		{
			// Ignore short events
			if ((EndTime - (*EventItr)->Start) > ContactEventMin)
			{
				// Set end time and publish event
				(*EventItr)->End = EndTime;
				//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White,
				//	FString::Printf(TEXT(" * * * * * * *BCAST* *EVENT* ")), false, FVector2D(1.5f, 1.5f));
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
}


// Event called when a semantic overlap event begins
void FSLManipulatorContactEventHandler::OnSLOverlapBegin(const FSLContactResult& SemanticOverlapResult)
{
	FSLManipulatorContactEventHandler::AddNewEvent(SemanticOverlapResult);
}

// Event called when a semantic overlap event ends
void FSLManipulatorContactEventHandler::OnSLOverlapEnd(UObject* Self, UObject* Other, float Time)
{
	FSLManipulatorContactEventHandler::FinishEvent(Other, Time);
}
