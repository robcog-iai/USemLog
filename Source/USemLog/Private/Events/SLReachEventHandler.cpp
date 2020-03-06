// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLReachEventHandler.h"
#include "SLEntitiesManager.h"
#include "SLReachListener.h"

// UUtils
#include "Ids.h"


// Set parent
void FSLReachEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Make sure the mappings singleton is initialized (the handler uses it)
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(InParent->GetWorld());
		}

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
void FSLReachEventHandler::OnSLPreAndReachEvent(const FSLEntity& Self, UObject* Other, float ReachStartTime, float ReachEndTime, float PreGraspEndTime)
{
	// Check that the objects are semantically annotated
	if (FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		const uint64 PairID =FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), OtherItem->Obj->GetUniqueID());
		if(ReachEndTime - ReachStartTime > ReachEventMin)
		{
			OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLReachEvent(
				FIds::NewGuidInBase64Url(), ReachStartTime, ReachEndTime,
				PairID,Self, *OtherItem)));
		}

		if(PreGraspEndTime - ReachEndTime > PreGraspPositioningEventMin)
		{
			OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLPreGraspPositioningEvent(
				FIds::NewGuidInBase64Url(), ReachEndTime, PreGraspEndTime,
				PairID,Self, *OtherItem)));
		}
	}
}
