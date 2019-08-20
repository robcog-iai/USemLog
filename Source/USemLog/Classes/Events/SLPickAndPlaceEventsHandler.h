// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "Events/SLLiftEvent.h"
#include "Events/SLSlideEvent.h"
#include "Events/SLTransportEvent.h"

/**
 * Listens to grasp events input, and outputs finished semantic grasp events
 */
class FSLPickAndPlaceEventsHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;

	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
	// Start new lift event
	void AddNewLiftEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime);

	// Finish then publish the event
	bool FinishLiftEvent(UObject* Other, float EndTime);

	// Start new lift event
	void AddNewSlideEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime);

	// Finish then publish the event
	bool FinishSlideEvent(UObject* Other, float EndTime);

	// Start new lift event
	void AddNewTransportEvent(const FSLEntity& Self, const FSLEntity& Other, float StartTime);

	// Finish then publish the event
	bool FinishTransportEvent(UObject* Other, float EndTime);
	
	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	
	// Event called when a semantic overlap event begins
	void OnSLLiftBegin(const FSLEntity& Self, UObject* Other, float Time);

	// Event called when a semantic overlap event ends
	void OnSLLiftEnd(const FSLEntity& Self, UObject* Other, float Time);

	// Event called when a semantic overlap event begins
	void OnSLSlideBegin(const FSLEntity& Self, UObject* Other, float Time);

	// Event called when a semantic overlap event ends
	void OnSLSlideEnd(const FSLEntity& Self, UObject* Other, float Time);

		// Event called when a semantic overlap event begins
	void OnSLTransportBegin(const FSLEntity& Self, UObject* Other, float Time);

	// Event called when a semantic overlap event ends
	void OnSLTransportEnd(const FSLEntity& Self, UObject* Other, float Time);
	
private:
	// Parent
	class USLPickAndPlaceListener* Parent;

	// Array of started events
	TArray<TSharedPtr<FSLLiftEvent>> StartedLiftEvents;
	TArray<TSharedPtr<FSLSlideEvent>> StartedSlideEvents;
	TArray<TSharedPtr<FSLTransportEvent>> StartedTransportEvents;
};
