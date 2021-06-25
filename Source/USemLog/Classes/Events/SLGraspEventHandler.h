// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLGraspEvent.h"

/**
 * Listens to grasp events input, and outputs finished semantic grasp events
 */
class FSLGraspEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;

	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
	// Start new grasp event
	void AddNewEvent(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, const FString& Type);

	// Finish then publish the event
	bool FinishEvent(USLBaseIndividual* Other, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& Type);

	// Event called when a semantic overlap event ends
	void OnSLGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);

private:
	// Parent
	class USLManipulatorMonitor* Parent;

	// Array of started events
	TArray<TSharedPtr<FSLGraspEvent>> StartedEvents;
	
	/* Constant values */
	constexpr static float GraspEventMin = 0.25f;
};
