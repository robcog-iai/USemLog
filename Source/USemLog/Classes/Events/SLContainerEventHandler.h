// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLGraspEvent.h"

/**
 * Listens to grasp events input, and outputs finished semantic grasp events
 */
class FSLContainerEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;

	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
	// Event called when a semantic overlap event begins
	void OnContainerManipulation(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, float EndTime, const FString& Type);

private:
	// Parent
	class USLContainerMonitor* Parent;
};
