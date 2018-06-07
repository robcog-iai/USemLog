// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/DummyEvent.h"

// Default constructor
FDummyEvent::FDummyEvent()
{
}


/* Begin IEvent interface */
// Get an owl representation of the event
FOwlNode FDummyEvent::ToOwlNode() const
{
	return FOwlNode();
}