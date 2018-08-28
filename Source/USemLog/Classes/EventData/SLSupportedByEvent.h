// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EventData/SLContactEvent.h"

/**
* Supported by event class
*/
class FSLSupportedByEvent : public FSLContactEvent
{
public:
	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FOwlDoc* OutDoc) override;
	/* End IEvent interface */
};