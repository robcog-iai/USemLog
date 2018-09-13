// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"

/**
* Dummy event class
*/

class FSLDummyEvent : public ISLEvent
{
public:
	// Default constructor
	FSLDummyEvent();

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FOwlDoc* OutDoc) override;

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;
	/* End IEvent interface */
};