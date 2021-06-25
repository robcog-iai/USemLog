// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLGraspEventHandler.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"

// Set parent
void FSLGraspEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLManipulatorMonitor>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLGraspEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe to the forwarded semantically annotated grasping broadcasts
		Parent->OnBeginManipulatorGrasp.AddRaw(this, &FSLGraspEventHandler::OnSLGraspBegin);
		Parent->OnEndManipulatorGrasp.AddRaw(this, &FSLGraspEventHandler::OnSLGraspEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLGraspEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent first publish any pending (delayed) events
		if(!Parent->IsFinished())
		{
			Parent->Finish();
		}
		
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

// Start new grasp event
void FSLGraspEventHandler::AddNewEvent(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, const FString& InType)
{
	// Start a semantic grasp event
	TSharedPtr<FSLGraspEvent> Event = MakeShareable(new FSLGraspEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime,
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other, InType));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLGraspEventHandler::FinishEvent(USLBaseIndividual* Other, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual == Other)
		{
			// Ignore short events
			if ((EndTime - (*EventItr)->StartTime) > GraspEventMin)
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

// Terminate and publish pending events (this usually is called at end play)
void FSLGraspEventHandler::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedEvents)
	{
		// Ignore short events
		if ((EndTime - Ev->StartTime) > GraspEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedEvents.Empty();
}


// Event called when a semantic grasp event begins
void FSLGraspEventHandler::OnSLGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& Type)
{
	AddNewEvent(Self, Other, Time, Type);
}

// Event called when a semantic grasp event ends
void FSLGraspEventHandler::OnSLGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishEvent(Other, Time);
}
