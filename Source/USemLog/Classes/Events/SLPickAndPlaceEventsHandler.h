// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"
#include "SLStructs.h"

/**
 * Listens to pick and place related events
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
	// Event called when a slide event happened
	void OnSLSlide(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime);

	// Event called when a pick up event happened
	void OnSLPickUp(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime);

	// Event called when a transport event happened
	void OnSLTransport(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime);

	// Event called when a put down event happened
	void OnSLPutDown(const FSLEntity& Self, AActor* Other, float StartTime, float EndTime);

private:
	// Parent
	class USLPickAndPlaceListener* Parent;
};

