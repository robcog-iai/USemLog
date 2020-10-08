// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLReachEventHandler.h"
#include "Monitors/SLReachListener.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"
#include "Utils/SLUuid.h"

// Set parent
void FSLReachEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLReachListener>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLReachEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe to the forwarded semantically annotated Reaching broadcasts
		Parent->OnPreGraspAndReachEvent.AddRaw(this, &FSLReachEventHandler::OnSLPreAndReachEvent);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLReachEventHandler::Finish(float EndTime, bool bForced)
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
void FSLReachEventHandler::OnSLPreAndReachEvent(USLBaseIndividual* Self, AActor* OtherActor, float ReachStartTime, float ReachEndTime, float PreGraspEndTime)
{
	// Check that the objects are semantically annotated
	if (USLBaseIndividual* OtherIndividual = FSLIndividualUtils::GetIndividualObject(OtherActor))
	{
		const uint64 PairID =FSLUuid::PairEncodeCantor(Self->GetUniqueID(), OtherIndividual->GetUniqueID());
		if(ReachEndTime - ReachStartTime > ReachEventMin)
		{
			OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLReachEvent(
				FSLUuid::NewGuidInBase64Url(), ReachStartTime, ReachEndTime,
				PairID, Self, OtherIndividual)));
		}

		if(PreGraspEndTime - ReachEndTime > PreGraspPositioningEventMin)
		{
			OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLPreGraspPositioningEvent(
				FSLUuid::NewGuidInBase64Url(), ReachEndTime, PreGraspEndTime,
				PairID, Self, OtherIndividual)));
		}
	}
}
