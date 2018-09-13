// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SLContactEvent.h"

/** Delegate for notification of finished semantic contact event */
DECLARE_DELEGATE_OneParam(FSLContactEventSignature, TSharedPtr<FSLContactEvent>);

// Forward declarations
struct FSLOverlapResult;

/**
 * Semantic contact publisher
 */
class FSLContactPublisher 
{
public:
	// Constructor 
	FSLContactPublisher(class USLOverlapArea* InParent);

	// Init
	void Init();

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime);

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
	void OnSLOverlapEnd(uint32 OtherId, float Time);

public:
	// Event called when a semantic contact event is finished
	FSLContactEventSignature OnSemanticContactEvent;

private:
	// Parent semantic overlap area
	class USLOverlapArea* Parent;

	// Array of started contact events
	TArray<TSharedPtr<FSLContactEvent>> StartedEvents;
};
