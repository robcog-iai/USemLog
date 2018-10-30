#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLSupportedByEvent.h"

// Forward declarations
struct FSLOverlapResult;

/**
 * Listens to contact events input, and outputs finished semantic supported-by events
 */
class FSLSupportedByEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;

	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime) override;

private:
	// Timer callback for creating events from the candidates list
	void InspectCandidatesCb();

	// Is a supported by event, returns nullptr if not an event
	bool IsPartOfASupportedByEvent(FSLOverlapResult& InCandidate, float Time, TSharedPtr<FSLSupportedByEvent> OutEvent);

	// Check if other obj is a supported by candidate
	bool IsACandidate(const uint32 InOtherId, bool bRemoveIfFound = false);

	// Finish then publish the event
	bool FinishEvent(const uint64 InPairId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLOverlapBegin(const FSLOverlapResult& InResult);

	// Event called when a semantic overlap event ends
	void OnSLOverlapEnd(uint32 OtherId, float Time);

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
	
	// Re-enable the overlap events in the next tick
	FTimerDelegate TimerDelegateNextTick;
};
