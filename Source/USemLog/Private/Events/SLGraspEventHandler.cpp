// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLGraspEventHandler.h"
#include "SLEntitiesManager.h"
#if SL_WITH_MC_GRASP
#include "MCFixationGrasp.h"
#endif // SL_WITH_MC_GRASP

// UUtils
#include "Ids.h"


// Set parent
void FSLGraspEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Make sure the mappings singleton is initialized (the handler uses it)
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(InParent->GetWorld());
		}

#if SL_WITH_MC_GRASP
		// Check if parent is of right type
		Parent = Cast<UMCFixationGrasp>(InParent);
#endif // SL_WITH_MC_GRASP

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
#if SL_WITH_MC_GRASP
		// Subscribe to the forwarded semantically annotated grasping broadcasts
		Parent->OnGraspBegin.AddRaw(this, &FSLGraspEventHandler::OnSLGraspBegin);
		Parent->OnGraspEnd.AddRaw(this, &FSLGraspEventHandler::OnSLGraspEnd);
#endif // SL_WITH_MC_GRASP

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLGraspEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		FSLGraspEventHandler::FinishAllEvents(EndTime);
	
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
void FSLGraspEventHandler::AddNewEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime)
{
	// Start a semantic grasp event
	TSharedPtr<FSLGraspEvent> Event = MakeShareable(new FSLGraspEvent(
		FIds::NewGuidInBase64Url(), StartTime, 
		FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other.Obj->GetUniqueID()),
		Self, Other));
	// Add event to the pending array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLGraspEventHandler::FinishEvent(UObject* Other, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Other.Obj == Other)
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

// Terminate and publish pending events (this usually is called at end play)
void FSLGraspEventHandler::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic grasp event begins
void FSLGraspEventHandler::OnSLGraspBegin(UObject* Self, UObject* Other, float Time)
{
	// Check that the objects are semantically annotated
	FSLEntity SelfItem = FSLEntitiesManager::GetInstance()->GetEntity(Self);
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(Other);
	if (SelfItem.IsSet() && OtherItem.IsSet())
	{
		FSLGraspEventHandler::AddNewEvent(SelfItem, OtherItem, Time);
	}
}

// Event called when a semantic grasp event ends
void FSLGraspEventHandler::OnSLGraspEnd(UObject* Self, UObject* Other, float Time)
{
	FSLGraspEventHandler::FinishEvent(Other, Time);
}
