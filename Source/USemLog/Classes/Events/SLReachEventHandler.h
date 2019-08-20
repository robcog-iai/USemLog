// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLReachEvent.h"

/**
 * Listens to Reach events input, and outputs finished semantic Reach events
 */
class FSLReachEventHandler : public ISLEventHandler
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
	void OnSLReachEvent(const FSLEntity& Self, UObject* Other, float StartTime, float EndTime);

private:
	// Parent
	class USLReachListener* Parent;

	// Array of started events
	TArray<TSharedPtr<FSLReachEvent>> StartedEvents;
};
