// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/SLGraspEvent.h"

/** Delegate for notification of finished semantic contact event */
DECLARE_DELEGATE_OneParam(FSLGraspEventSignature, TSharedPtr<FSLGraspEvent>);

// Forward declarations
struct FSLGraspResult;

/**
 * Semantic grasp publisher
 */
class FSLGraspPublisher 
{
public:
	// Constructor 
	FSLGraspPublisher(class USLGraspTrigger* InParent);

	// Init
	void Init();

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime);

private:
	// Start new event
	void AddNewEvent(const FSLGraspResult& SemanticOverlapBeginResult);

	// Finish then publish the event
	bool FinishEvent(const uint32 InOtherId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLGraspBegin(const FSLGraspResult& GraspBeginResult);
	
	// Event called when a semantic overlap event ends
	void OnSLGraspEnd(uint32 OtherId, float Time);

public:
	// Event called when a semantic grasp event is finished
	FSLGraspEventSignature OnSemanticGraspEvent;

private:
	// Parent
	class USLGraspTrigger* Parent;

	// Array of started events
	TArray<TSharedPtr<FSLGraspEvent>> StartedEvents;
};
