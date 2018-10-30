// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLGraspEventHandler.h"

#if WITH_MC_GRASP
#include "MCFixationGrasp.h"
#endif // WITH_MC_GRASP

// UUtils
#include "Ids.h"

// Set parent
void FSLGraspEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
#if WITH_MC_GRASP
		// Check if parent is of right type
		Parent = Cast<UMCFixationGrasp>(InParent);
#endif // WITH_MC_GRASP

		if (Parent)
		{
			UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
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
#if WITH_MC_GRASP
		// Subscribe to the forwarded semantically annotated grasping broadcasts
		Parent->OnGraspBegin.AddRaw(this, &FSLGraspEventHandler::OnSLGraspBegin);
		Parent->OnGraspEnd.AddRaw(this, &FSLGraspEventHandler::OnSLGraspEnd);
#endif // WITH_MC_GRASP

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLGraspEventHandler::Finish(float EndTime)
{
	if (bIsStarted || bIsInit)
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

//// Start new grasp event
//void FSLGraspEventHandler::AddNewEvent(const FSLGraspResult& InBeginResult)
//{
//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \t\t\t ADD NEW SL GRASP"), TEXT(__FUNCTION__), __LINE__);
//	// Start a semantic grasp event
//	TSharedPtr<FSLGraspEvent> Event = MakeShareable(new FSLGraspEvent(
//		FIds::NewGuidInBase64Url(), InBeginResult.TriggerTime, FIds::PairEncodeCantor(InBeginResult.Id, Parent->OwnerId),
//		Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass,
//		InBeginResult.Id, InBeginResult.SemId, InBeginResult.SemClass));
//	// Add event to the pending array
//	StartedEvents.Emplace(Event);
//}

// Publish finished event
bool FSLGraspEventHandler::FinishEvent(const uint32 InOtherId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->OtherId == InOtherId)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \t\t\t FINISH SL GRASP"), TEXT(__FUNCTION__), __LINE__);
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
void FSLGraspEventHandler::OnSLGraspBegin(uint32 SelfId, uint32 OtherId, float Time)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	//FSLGraspEventHandler::AddNewEvent(GraspBeginResult);
}

// Event called when a semantic grasp event ends
void FSLGraspEventHandler::OnSLGraspEnd(uint32 SelfId, uint32 OtherId, float Time)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	//FSLGraspEventHandler::FinishEvent(OtherId, Time);
}
