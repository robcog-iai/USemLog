// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLSlicingEvent.h"

/**
 * Listens to Slicing events input, and outputs finished semantic Slicing events
 */
class FSLSlicingEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;

	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
	// Start new Slicing event
	void AddNewEvent(const FSLEntity& PerformedBy, const FSLEntity& DeviceUsed, const FSLEntity& ObjectActedOn, float StartTime);

	// Finish then publish the event
	bool FinishEvent(UObject* InObjectActedOn, bool bTaskSuccessful, float EndTime, const FSLEntity& OutputsCreated);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a slicing event begins
	void OnSLSlicingBegin(UObject* PerformedBy, UObject* DeviceUsed, UObject* ObjectActedOn, float Time);
	
	// Event called when a slicing event ends unsuccessfuly
	void OnSLSlicingEndFail(UObject* PerformedBy, UObject* ObjectActedOn, float Time);

	// Event called when a slicing event ends successfuly
	void OnSLSlicingEndSuccess(UObject* PerformedBy, UObject* ObjectActedOn, UObject* OutputsCreated, float Time);

	// Event called when new objects are created
	void OnSLObjectCreation(UObject* TransformedObject, UObject* NewSlice, float Time);

	// Event called when an object is destroyed
	void OnSLObjectDestruction(UObject* ObjectActedOn, float Time);

private:
	// Parent
#if SL_WITH_SLICING
	class USlicingBladeComponent* Parent;
#else
	UObject* Parent;
#endif // SL_WITH_Slicing

	// Array of started events
	TArray<TSharedPtr<FSLSlicingEvent>> StartedEvents;
};
