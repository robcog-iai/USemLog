// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/DummyEvent.h"

// Default constructor
FDummyEvent::FDummyEvent()
{
	Start = 0.2f;
	End = 1231.f;
}


/* Begin IEvent interface */
// Get an owl representation of the event
FOwlNode FDummyEvent::ToOwlNode() const
{
	return FOwlNode();
}