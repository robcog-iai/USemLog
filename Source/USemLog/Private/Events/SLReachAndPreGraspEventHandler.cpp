// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLReachAndPreGraspEventHandler.h"
#include "Monitors/SLReachAndPreGraspMonitor.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"

// Set parent
void FSLReachAndPreGraspEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLReachAndPreGraspMonitor>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLReachAndPreGraspEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe to the forwarded semantically annotated Reaching broadcasts
		Parent->OnReachAndPreGraspEvent.AddRaw(this, &FSLReachAndPreGraspEventHandler::OnSLReachAndPreGraspEvent);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLReachAndPreGraspEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent finish first
		if(!Parent->IsFinished())
		{
			Parent->Finish();
		}
		
		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Event called when a semantic Reach event begins
void FSLReachAndPreGraspEventHandler::OnSLReachAndPreGraspEvent(USLBaseIndividual* Self, USLBaseIndividual* Other, float ReachStartTime, float ReachEndTime, float PreGraspEndTime)
{
	const uint64 PairID =FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID());
	if(ReachEndTime - ReachStartTime > ReachEventMin)
	{
		TSharedPtr<FSLReachEvent> Event = MakeShareable(new FSLReachEvent(
			FSLUuid::NewGuidInBase64Url(), ReachStartTime, ReachEndTime,
			PairID, Self, Other));
		Event->EpisodeId = EpisodeId;
		OnSemanticEvent.ExecuteIfBound(Event);
	}

	if(PreGraspEndTime - ReachEndTime > PreGraspEventMin)
	{
		TSharedPtr<FSLPreGraspEvent> Event = MakeShareable(new FSLPreGraspEvent(
			FSLUuid::NewGuidInBase64Url(), ReachEndTime, PreGraspEndTime,
			PairID, Self, Other));
		Event->EpisodeId = EpisodeId;
		OnSemanticEvent.ExecuteIfBound(Event);
	}
}
