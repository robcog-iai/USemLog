// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
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
	void Finish(float EndTime) override;

private:
	//// Start new event
	//void AddNewEvent(uint32 SelfId, uint32 OtherId, float /*Time*/);

	// Finish then publish the event
	bool FinishEvent(const uint32 InOtherId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLGraspBegin(uint32 SelfId, uint32 OtherId, float Time);
	
	// Event called when a semantic overlap event ends
	void OnSLGraspEnd(uint32 SelfId, uint32 OtherId, float Time);

private:
	// Parent
#if WITH_MC_GRASP
	class UMCFixationGrasp* Parent;
#else
	UObject* Parent;
#endif // WITH_MC_GRASP

	// Array of started events
	TArray<TSharedPtr<FSLGraspEvent>> StartedEvents;
};
