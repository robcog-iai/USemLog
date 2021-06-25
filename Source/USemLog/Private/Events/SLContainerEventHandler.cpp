// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContainerEventHandler.h"
#include "Monitors/SLContainerMonitor.h"
#include "Individuals/SLIndividualUtils.h"
#include "Events/SLContainerEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"


// Set parent
void FSLContainerEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLContainerMonitor>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLContainerEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe to the forwarded semantically annotated grasping broadcasts
		Parent->OnContainerManipulation.AddRaw(this, &FSLContainerEventHandler::OnContainerManipulation);

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLContainerEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{

		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Event called when a semantic grasp happens
void FSLContainerEventHandler::OnContainerManipulation(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, float EndTime, const FString& Type)
{
	OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLContainerEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EndTime,
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other, Type)));
}
