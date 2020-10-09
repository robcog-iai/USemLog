// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSlicingEventHandler.h"
#include "Utils/SLUuid.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/SLIndividualUtils.h"

#if SL_WITH_SLICING
#include "SlicingBladeComponent.h"
#endif // SL_WITH_SLICING



// Set parent
void FSLSlicingEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
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
void FSLSlicingEventHandler::AddNewEvent(USLBaseIndividual* PerformedBy, USLBaseIndividual* DeviceUsed, USLBaseIndividual* ObjectActedOn, float StartTime)
{
	// Start a semantic Slicing event
	TSharedPtr<FSLSlicingEvent> Event = MakeShareable(new FSLSlicingEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, 
		FSLUuid::PairEncodeCantor(PerformedBy->GetUniqueID(), ObjectActedOn->GetUniqueID()),
		PerformedBy, DeviceUsed, ObjectActedOn));
	// Add event to the pending array
	StartedEvents.Emplace(Event);
}

// Publish finished event
bool FSLSlicingEventHandler::FinishEvent(USLBaseIndividual* ObjectActedOn,
	bool bInTaskSuccessful, float EndTime,
	USLBaseIndividual* OutputsCreated)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->ObjectActedOn == ObjectActedOn)
		{
			// Set end time and publish event
			(*EventItr)->EndTime = EndTime;
			(*EventItr)->bTaskSuccessful = bInTaskSuccessful;
			(*EventItr)->CreatedSlice = OutputsCreated;
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
		Ev->EndTime = EndTime;
		OnSemanticEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic Slicing event begins
void FSLSlicingEventHandler::OnSLSlicingBegin(AActor* PerformedBy, AActor* DeviceUsed, AActor* ObjectActedOn, float Time)
{
	if (USLBaseIndividual* PerformedByIndvidiual = FSLIndividualUtils::GetIndividualObject(PerformedBy))
	{
		if (USLBaseIndividual* DeviceUsedIndividual = FSLIndividualUtils::GetIndividualObject(DeviceUsed))
		{
			if (USLBaseIndividual* ObjectActedOnIndividual = FSLIndividualUtils::GetIndividualObject(ObjectActedOn))
			{
				FSLSlicingEventHandler::AddNewEvent(PerformedByIndvidiual, DeviceUsedIndividual, ObjectActedOnIndividual, Time);
			}
		}
	}
}

// Event called when a semantic Slicing event ends
void FSLSlicingEventHandler::OnSLSlicingEndFail(AActor* PerformedBy, AActor* ObjectActedOn, float Time)
{
	if (USLBaseIndividual* PerformedByIndvidiual = FSLIndividualUtils::GetIndividualObject(PerformedBy))
	{
		if (USLBaseIndividual* ObjectActedOnIndvidiual = FSLIndividualUtils::GetIndividualObject(ObjectActedOn))
		{
			FSLSlicingEventHandler::FinishEvent(ObjectActedOnIndvidiual, false, Time, nullptr);
		}
	}
}

// Event called when a semantic Slicing event ends
void FSLSlicingEventHandler::OnSLSlicingEndSuccess(AActor* PerformedBy, AActor* ObjectActedOn, AActor* ObjectCreated, float Time)
{
	if (USLBaseIndividual* PerformedByIndvidiual = FSLIndividualUtils::GetIndividualObject(PerformedBy))
	{
		if (USLBaseIndividual* ObjectActedOnIndividual = FSLIndividualUtils::GetIndividualObject(ObjectActedOn))
		{
			if (USLBaseIndividual* ObjectCreatedIndividual = FSLIndividualUtils::GetIndividualObject(ObjectCreated))
			{
				FSLSlicingEventHandler::FinishEvent(ObjectActedOnIndividual, true, Time, ObjectCreatedIndividual);
			}
		}
	}
}

// Event called when new objects are created
void FSLSlicingEventHandler::OnSLObjectCreation(AActor* TransformedObject, AActor* NewSlice, float Time)
{
	if (USLBaseIndividual* OrigIndividual = FSLIndividualUtils::GetIndividualObject(TransformedObject))
	{
		// TODO create a new if since it is a new object
		// Hide / remove previous whole component
	}

	// TODO Create a new slice individual
}

// Event called when an object is destroyed
void FSLSlicingEventHandler::OnSLObjectDestruction(AActor* ObjectActedOn, float Time)
{
	if (USLBaseIndividual* ActedOnIndividual = FSLIndividualUtils::GetIndividualObject(ObjectActedOn))
	{
		// TODO hide or remove individual
		UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	}

}
