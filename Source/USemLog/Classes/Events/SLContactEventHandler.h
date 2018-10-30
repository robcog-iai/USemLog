// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLContactEvent.h"

// Forward declarations
struct FSLOverlapResult;

/**
 * Listens to contact events input, and outputs finished semantic contact events
 */
class FSLContactEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;
	
	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime) override;

private:
	// Start new contact event
	void AddNewEvent(const FSLOverlapResult& InResult);

	// Finish then publish the event
	bool FinishEvent(const uint32 InOtherId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLOverlapBegin(const FSLOverlapResult& InResult);
	
	// Event called when a semantic overlap event ends
	void OnSLOverlapEnd(uint32 SelfId, uint32 OtherId, float Time);

private:
	// Parent semantic overlap area
	class USLOverlapArea* Parent;

	// Array of started contact events
	TArray<TSharedPtr<FSLContactEvent>> StartedEvents;
};
