// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPickAndPlaceEventsHandler.h"
#include "SLEntitiesManager.h"
#include "SLPickAndPlaceListener.h"

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


		//// Subscribe to the forwarded semantically annotated grasping broadcasts
		//Parent->OnBeginManipulatorLift.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLLiftBegin);
		//Parent->OnEndManipulatorLift.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLLiftEnd);

		//Parent->OnManipulatorSlideEvent.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLSlideBegin);
		//Parent->OnEndManipulatorSlide.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLSlideEnd);

		//Parent->OnBeginManipulatorTransport.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLTransportBegin);
		//Parent->OnEndManipulatorTransport.AddRaw(this, &FSLPickAndPlaceEventsHandler::OnSLTransportEnd);
		
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

// Start new event
void FSLPickAndPlaceEventsHandler::AddNewLiftEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime)
{
	// Start a semantic grasp event
	TSharedPtr<FSLPickUpEvent> Event = MakeShareable(new FSLPickUpEvent(
		FIds::NewGuidInBase64Url(), StartTime,
		FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other.Obj->GetUniqueID()),
		Self, Other));
	// Add event to the pending array
	StartedLiftEvents.Emplace(Event);
}

// Publish finished event
bool FSLPickAndPlaceEventsHandler::FinishLiftEvent(UObject* Other, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedLiftEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Item.Obj == Other)
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

// Start new event
void FSLPickAndPlaceEventsHandler::AddNewSlideEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime)
{
	// Start a semantic grasp event
	TSharedPtr<FSLSlideEvent> Event = MakeShareable(new FSLSlideEvent(
		FIds::NewGuidInBase64Url(), StartTime,
		FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other.Obj->GetUniqueID()),
		Self, Other));
	// Add event to the pending array
	StartedSlideEvents.Emplace(Event);
}

// Publish finished event
bool FSLPickAndPlaceEventsHandler::FinishSlideEvent(UObject* Other, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedSlideEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Item.Obj == Other)
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


// Start new event
void FSLPickAndPlaceEventsHandler::AddNewTransportEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime)
{
	// Start a semantic grasp event
	TSharedPtr<FSLTransportEvent> Event = MakeShareable(new FSLTransportEvent(
		FIds::NewGuidInBase64Url(), StartTime,
		FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other.Obj->GetUniqueID()),
		Self, Other));
	// Add event to the pending array
	StartedTransportEvents.Emplace(Event);
}

// Publish finished event
bool FSLPickAndPlaceEventsHandler::FinishTransportEvent(UObject* Other, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedTransportEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Item.Obj == Other)
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
void FSLPickAndPlaceEventsHandler::FinishAllEvents(float EndTime)
{
	// Finish events
	for (auto& Ev : StartedLiftEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);

	}
	StartedLiftEvents.Empty();

	for (auto& Ev : StartedSlideEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedLiftEvents.Empty();

	for (auto& Ev : StartedLiftEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedLiftEvents.Empty();
}


// Event called when a semantic grasp event begins
void FSLPickAndPlaceEventsHandler::OnSLPickUp(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime)
{
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d PickUp event sent.."), *FString(__func__), __LINE__);

		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLPickUpEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem)));
	}
}


// Event called when a semantic grasp event begins
void FSLPickAndPlaceEventsHandler::OnSLSlide(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime)
{
	if(FSLEntity* OtherItem = FSLEntitiesManager::GetInstance()->GetEntityPtr(Other))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Slide event sent.."), *FString(__func__), __LINE__);

		OnSemanticEvent.ExecuteIfBound(MakeShareable(new FSLSlideEvent(
			FIds::NewGuidInBase64Url(), StartTime, EndTime,
			FIds::PairEncodeCantor(Self.Obj->GetUniqueID(), Other->GetUniqueID()),
			Self, *OtherItem)));
	}
}

// Event called when a semantic grasp event begins
void FSLPickAndPlaceEventsHandler::OnSLTransportBegin(const FSLEntity& Self, AActor* Other, float Time)
{
	// Check that the objects are semantically annotated
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(Other);
	if (OtherItem.IsSet())
	{
		FSLPickAndPlaceEventsHandler::AddNewTransportEvent(Self, OtherItem, Time);
	}
}

// Event called when a semantic grasp event ends
void FSLPickAndPlaceEventsHandler::OnSLTransportEnd(const FSLEntity& Self, AActor* Other, float Time)
{
	FSLPickAndPlaceEventsHandler::FinishTransportEvent(Other, Time);
}