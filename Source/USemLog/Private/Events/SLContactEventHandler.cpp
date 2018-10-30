// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEventHandler.h"
#include "SLOverlapArea.h"

// UUtils
#include "Ids.h"

// Set parent
void FSLContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{		// Check if parent is of right type
		Parent = Cast<USLOverlapArea>(InParent);
		if (Parent)
		{
			UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLContactEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
		Parent->OnBeginSLOverlap.AddRaw(this, &FSLContactEventHandler::OnSLOverlapBegin);
		Parent->OnEndSLOverlap.AddRaw(this, &FSLContactEventHandler::OnSLOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLContactEventHandler::Finish(float EndTime)
{
	if (bIsStarted || bIsInit)
	{
		FSLContactEventHandler::FinishAllEvents(EndTime);
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

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
void FSLContactEventHandler::AddNewEvent(const FSLOverlapResult& InResult)
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
bool FSLContactEventHandler::FinishEvent(const uint32 InOtherId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Obj2Id == InOtherId)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			OnSemanticEvent.ExecuteIfBound(*EventItr);
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
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic overlap event begins
void FSLContactEventHandler::OnSLOverlapBegin(const FSLOverlapResult& SemanticOverlapResult)
{
	FSLContactEventHandler::AddNewEvent(SemanticOverlapResult);
}

// Event called when a semantic overlap event ends
void FSLContactEventHandler::OnSLOverlapEnd(uint32 OtherId, float Time)
{
	FSLContactEventHandler::FinishEvent(OtherId, Time);
}
