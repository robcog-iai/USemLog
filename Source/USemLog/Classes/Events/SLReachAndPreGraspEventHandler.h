// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLReachEvent.h"
#include "Events/SLPreGraspEvent.h"

// Forward declaraion
class AActor;

/**
 * Listens to Reach events input, and outputs finished semantic Reach events
 */
class FSLReachAndPreGraspEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;

	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
	// Create and publish finished reach event
	void OnSLReachAndPreGraspEvent(USLBaseIndividual* Self, USLBaseIndividual* Other, float ReachStartTime, float ReachEndTime, float PreGraspEndTime);

private:
	// Parent
	class USLReachAndPreGraspMonitor* Parent;

	/* Constants */
	// Minimal duration for the reaching events
	constexpr static float ReachEventMin = 0.25f;
	// Minimal duration for the positioning events
	constexpr static float PreGraspEventMin = 0.2f;
};
