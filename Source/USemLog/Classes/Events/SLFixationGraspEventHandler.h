// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLGraspEvent.h"

/**
 * Listens to fixation grasp events input, and outputs finished semantic grasp events
 */
class FSLFixationGraspEventHandler : public ISLEventHandler
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
	void AddNewEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime);

	// Finish then publish the event
	bool FinishEvent(UObject* InOther, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLGraspBegin(UObject* Self, UObject* Other, float Time);
	
	// Event called when a semantic overlap event ends
	void OnSLGraspEnd(UObject* Self, UObject* Other, float Time);

private:
	// Parent
#if SL_WITH_MC_GRASP
	class UMCGraspFixation* Parent;
#else
	UObject* Parent;
#endif // SL_WITH_MC_GRASP

	// Array of started events
	TArray<TSharedPtr<FSLGraspEvent>> StartedEvents;
};
