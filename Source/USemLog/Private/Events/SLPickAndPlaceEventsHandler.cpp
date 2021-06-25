// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPickAndPlaceEventsHandler.h"
#include "Monitors/SLPickAndPlaceMonitor.h"
#include "Events/SLPickUpEvent.h"
#include "Events/SLSlideEvent.h"
#include "Events/SLPutDownEvent.h"
#include "Events/SLTransportEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"

// Set parent
void FSLPickAndPlaceEventsHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<USLPickAndPlaceMonitor>(InParent);
		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLPickAndPlaceEventsHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		Parent->OnManipulatorPickUpEvent.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLPickUp);
		Parent->OnManipulatorSlideEvent.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLSlide);
		Parent->OnManipulatorTransportEvent.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLTransport);
		Parent->OnManipulatorPutDownEvent.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLPutDown);
		
		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLPickAndPlaceEventsHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent finish first
		if(!Parent->IsFinished())
		{
			Parent->Finish(EndTime);
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

// Event called when a slide event happened
void FSLPickAndPlaceEventsHandler::OnSLSlide(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, float EndTime)
{
	TSharedPtr<FSLSlideEvent> Event = MakeShareable(new FSLSlideEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EndTime,
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other));
	Event->EpisodeId = EpisodeId;
	OnSemanticEvent.ExecuteIfBound(Event);
}

// Event called when a pick up event happened
void FSLPickAndPlaceEventsHandler::OnSLPickUp(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, float EndTime)
{
	TSharedPtr<FSLPickUpEvent> Event = MakeShareable(new FSLPickUpEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EndTime,
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other));

	Event->EpisodeId = EpisodeId;
	OnSemanticEvent.ExecuteIfBound(Event);
}

// Event called when a transport event happened
void FSLPickAndPlaceEventsHandler::OnSLTransport(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, float EndTime)
{
	TSharedPtr<FSLTransportEvent> Event = MakeShareable(new FSLTransportEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EndTime,
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other));

	Event->EpisodeId = EpisodeId;
	OnSemanticEvent.ExecuteIfBound(Event);
}

// Event called when a put down event happened
void FSLPickAndPlaceEventsHandler::OnSLPutDown(USLBaseIndividual* Self,USLBaseIndividual* Other, float StartTime, float EndTime)
{
	TSharedPtr<FSLPutDownEvent> Event = MakeShareable(new FSLPutDownEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EndTime,
		FSLUuid::PairEncodeCantor(Self->GetUniqueID(), Other->GetUniqueID()),
		Self, Other));

	Event->EpisodeId = EpisodeId;
	OnSemanticEvent.ExecuteIfBound(Event);
}
