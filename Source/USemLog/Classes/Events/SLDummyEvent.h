// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
	FSLDummyEvent() = default;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;

	// Get the event type name
	virtual FString TypeName() const override { return FString(TEXT("Dummy")); };
	/* End IEvent interface */
};