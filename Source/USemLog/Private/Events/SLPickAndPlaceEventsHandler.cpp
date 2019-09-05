// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPickAndPlaceEventsHandler.h"
#include "SLEntitiesManager.h"
#include "SLPickAndPlaceListener.h"

#include "Events/SLPickUpEvent.h"
#include "Events/SLSlideEvent.h"
#include "Events/SLPutDownEvent.h"
#include "Events/SLTransportEvent.h"

// UUtils
#include "Ids.h"


// Set parent
void FSLPickAndPlaceEventsHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Make sure the mappings singleton is initialized (the handler uses it)
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(InParent->GetWorld());
		}

		// Check if parent is of right type
		Parent = Cast<USLPickAndPlaceListener>(InParent);
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



// Event called when a slide event happened
void FSLPickAndPlaceEventsHandler::OnSLSlide(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime)
{
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLSlideEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem)));
	}
}

// Event called when a pick up event happened
void FSLPickAndPlaceEventsHandler::OnSLPickUp(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime)
{
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLPickUpEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem)));
	}
}

// Event called when a transport event happened
void FSLPickAndPlaceEventsHandler::OnSLTransport(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime)
{
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLTransportEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem)));
	}
}

// Event called when a put down event happened
void FSLPickAndPlaceEventsHandler::OnSLPutDown(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime)
{
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLPutDownEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem)));
	}
}
