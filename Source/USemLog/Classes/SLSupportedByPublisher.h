#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EventData/SLSupportedByEvent.h"

/** Delegate for notification of finished semantic contact event */
DECLARE_DELEGATE_OneParam(FSLSupportedByEventSignature, TSharedPtr<FSLSupportedByEvent>);

// Forward declarations
struct FSLOverlapResult;

/**
 * Supported by publisher
 */
class FSLSupportedByPublisher
{
public:
	// Constructor 
	FSLSupportedByPublisher(class USLOverlapArea* InSLOverlapArea);

	// Init
	void Init();

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime);

private:
	// Timer callback for creating events from the candidates list
	void AddEventsTimerCb();

	// Check if other obj is a supported by candidate
	bool IsACandidate(const uint32 InOtherId, bool bRemoveIfFound = false);

	// Finish then publish the event
	bool FinishEvent(const uint64 InPairId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLOverlapBegin(const FSLOverlapResult& SemanticOverlapBeginResult);

	// Event called when a semantic overlap event ends
	void OnSLOverlapEnd(const FSLOverlapResult& SemanticOverlapEndResult);

public:
	// Event called when a supported by event is finished
	FSLSupportedByEventSignature OnSupportedByEvent;

private:
	// Parent semantic overlap area
	class USLOverlapArea* Parent;
	
	// Candidates for supported by event
	TArray<FSLOverlapResult> Candidates;

	// Array of started supported by events
	TArray<TSharedPtr<FSLSupportedByEvent>> StartedEvents;

	// Timer handle to trigger callback to check if candidates are in a supported by event
	FTimerHandle TimerHandle;

	// Timer delegate to be able to bind against non UObject functions
	FTimerDelegate TimerDelegate;



	//////////////////////////////////////////////////////////////////////////
	//// Check if a supported by event started from the candidates
	//void SupportedByTimerCb();

	////
	//FTimerDelegate TimerDelegateNextTick;
	//void NextTickCb();
};
