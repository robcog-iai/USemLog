// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSlicingEventHandler.h"
#include "SLMappings.h"
#include "Tags.h"
#if SL_WITH_SLICING
#include "SlicingBladeComponent.h"
#endif // SL_WITH_SLICING

// UUtils
#include "Ids.h"


// Set parent
void FSLSlicingEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Make sure the mappings singleton is initialized (the handler uses it)
		if (!FSLMappings::GetInstance()->IsInit())
		{
			FSLMappings::GetInstance()->Init(InParent->GetWorld());
		}

#if SL_WITH_SLICING
		// Check if parent is of right type
		Parent = Cast<USlicingBladeComponent>(InParent);
#endif // SL_WITH_SLICING

		if (Parent)
		{
			// Mark as initialized
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLSlicingEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
#if SL_WITH_SLICING
		// Subscribe to the forwarded semantically annotated Slicing broadcasts
		Parent->OnBeginSlicing.AddRaw(this, &FSLSlicingEventHandler::OnSLSlicingBegin);
		Parent->OnEndSlicingFail.AddRaw(this, &FSLSlicingEventHandler::OnSLSlicingEndFail);
		Parent->OnEndSlicingSuccess.AddRaw(this, &FSLSlicingEventHandler::OnSLSlicingEndSuccess);
		Parent->OnObjectCreation.AddRaw(this, &FSLSlicingEventHandler::OnSLObjectCreation);
		Parent->OnObjectDestruction.AddRaw(this, &FSLSlicingEventHandler::OnSLObjectDestruction);
#endif // SL_WITH_SLICING

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLSlicingEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		FSLSlicingEventHandler::FinishAllEvents(EndTime);
	
		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject		

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Start new Slicing event
void FSLSlicingEventHandler::AddNewEvent(const FSLItem& PerformedBy, const FSLItem& DeviceUsed, const FSLItem& ObjectActedOn, float StartTime)
{
	// Start a semantic Slicing event
	TSharedPtr<FSLSlicingEvent> Event = MakeShareable(new FSLSlicingEvent(
		FIds::NewGuidInBase64Url(), StartTime, 
		FIds::PairEncodeCantor(PerformedBy.Obj->GetUniqueID(), ObjectActedOn.Obj->GetUniqueID()),
		PerformedBy, DeviceUsed, ObjectActedOn));
	// Add event to the pending array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLSlicingEventHandler::FinishEvent(UObject* ObjectActedOn, bool taskSuccess, float EndTime, const FSLItem& OutputsCreated = FSLItem::FSLItem())
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->ObjectActedOn.Obj == ObjectActedOn)
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			(*EventItr)->TaskSuccess = taskSuccess;
			(*EventItr)->OutputsCreated = OutputsCreated;
			OnSemanticEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending events (this usually is called at end play)
void FSLSlicingEventHandler::FinishAllEvents(float EndTime)
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


// Event called when a semantic Slicing event begins
void FSLSlicingEventHandler::OnSLSlicingBegin(UObject* PerformedBy, UObject* DeviceUsed, UObject* ObjectActedOn, float Time)
{
	// Check that the objects are semantically annotated
  	FSLItem PerformedByItem = FSLMappings::GetInstance()->GetItem(PerformedBy);
	FSLItem DeviceUsedItem = FSLMappings::GetInstance()->GetItem(DeviceUsed);
	FSLItem CutItem = FSLMappings::GetInstance()->GetItem(ObjectActedOn);
	if (PerformedByItem.IsValid()
		&& CutItem.IsValid()
		&& DeviceUsedItem.IsValid())
	{
		FSLSlicingEventHandler::AddNewEvent(PerformedByItem, DeviceUsedItem, CutItem, Time);
	}
}

// Event called when a semantic Slicing event ends
void FSLSlicingEventHandler::OnSLSlicingEndFail(UObject* PerformedBy, UObject* ObjectActedOn, float Time)
{
	FSLItem PerformedByItem = FSLMappings::GetInstance()->GetItem(PerformedBy);
	FSLItem CutItem = FSLMappings::GetInstance()->GetItem(ObjectActedOn);
	if (PerformedByItem.IsValid()
		&& CutItem.IsValid())
	{
		FSLSlicingEventHandler::FinishEvent(ObjectActedOn, false, Time);
	}
}

// Event called when a semantic Slicing event ends
void FSLSlicingEventHandler::OnSLSlicingEndSuccess(UObject* PerformedBy, UObject* ObjectActedOn, UObject* OutputsCreated, float Time)
{
	FSLItem PerformedByItem = FSLMappings::GetInstance()->GetItem(PerformedBy);
	FSLItem CutItem = FSLMappings::GetInstance()->GetItem(ObjectActedOn);
	FSLItem OutputsCreatedItem = FSLMappings::GetInstance()->GetItem(OutputsCreated);
	if (PerformedByItem.IsValid()
		&& CutItem.IsValid()
		&& OutputsCreatedItem.IsValid())
	{
		FSLSlicingEventHandler::FinishEvent(ObjectActedOn, true, Time, OutputsCreatedItem);
	}
}

// Event called when new objects are created
void FSLSlicingEventHandler::OnSLObjectCreation(UObject* TransformedObject, UObject* NewSlice, float Time)
{
	// Only create Id for new Slice and use same old ID for Original object
	FString IdValue = FIds::NewGuidInBase64() + "_";
	IdValue.Append(FTags::GetKeyValue(TransformedObject, "SemLog", "Id"));
	FTags::AddKeyValuePair(NewSlice, "SemLog", "Id", IdValue, true);

	if (FSLMappings::GetInstance()->AddItem(TransformedObject) &&
		FSLMappings::GetInstance()->AddItem(NewSlice))
	{
		UE_LOG(LogTemp, Error, TEXT(">>Items Have been Created"));
	}
}

// Event called when an object is destroyed
void FSLSlicingEventHandler::OnSLObjectDestruction(UObject* ObjectActedOn, float Time)
{
	FSLItem OtherItem = FSLMappings::GetInstance()->GetItem(ObjectActedOn);
	if (OtherItem.IsValid())
	{
		FSLMappings::GetInstance()->RemoveItem(ObjectActedOn);
	}
}
