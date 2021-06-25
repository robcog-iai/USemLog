// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLFixationGraspEventHandler.h"
#include "Events/SLGraspEvent.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"

#if SL_WITH_MC_GRASP
#include "MCGraspFixation.h"
#endif // SL_WITH_MC_GRASP



// Set parent
void FSLFixationGraspEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
#if SL_WITH_MC_GRASP
		// Check if parent is of right type
		Parent = Cast<UMCGraspFixation>(InParent);
#endif // SL_WITH_MC_GRASP

		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLFixationGraspEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
#if SL_WITH_MC_GRASP
		// Subscribe to the forwarded semantically annotated grasping broadcasts
		Parent->OnGraspBegin.AddRaw(this, &FSLFixationGraspEventHandler::OnSLGraspBegin);
		Parent->OnGraspEnd.AddRaw(this, &FSLFixationGraspEventHandler::OnSLGraspEnd);
#endif // SL_WITH_MC_GRASP

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLFixationGraspEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		FinishAllEvents(EndTime);
	
		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from USLBaseIndividual		

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Start new grasp event
void FSLFixationGraspEventHandler::AddNewEvent(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime)
{
	// Start a semantic grasp event
	TSharedPtr<FSLGraspEvent> Event = MakeShareable(new FSLGraspEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, 
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLFixationGraspEventHandler::FinishEvent(USLBaseIndividual* Other, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual == Other)
		{
			// Set end time and publish event
			(*EventItr)->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending events (this usually is called at end play)
void FSLFixationGraspEventHandler::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->EndTime = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic grasp event begins
void FSLFixationGraspEventHandler::OnSLGraspBegin(AActor* SelfActor, AActor* OtherActor, float Time)
{
	// Check that the objects are semantically annotated
	if (USLBaseIndividual* SelfIndividual = FSLIndividualUtils::GetIndividualObject(SelfActor))
	{
		if (USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor))
		{
			FSLFixationGraspEventHandler::AddNewEvent(SelfIndividual, OtherIndividual, Time);
		}
	}
}

// Event called when a semantic grasp event ends
void FSLFixationGraspEventHandler::OnSLGraspEnd(AActor* SelfActor, AActor* OtherActor, float Time)
{
	if (USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor))
	{
		FSLFixationGraspEventHandler::FinishEvent(OtherIndividual, Time);
	}
}
