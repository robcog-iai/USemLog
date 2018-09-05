// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLGraspEvent.h"

// Default constructor
FSLGraspEvent::FSLGraspEvent()
{
	Start = 0.2f;
	End = 1231.f;
}


/* Begin ISLEvent interface */
// Get an owl representation of the event
FOwlNode FSLGraspEvent::ToOwlNode(/*ESLEventsTemplate TemplateType*/) const
{
	return FOwlNode();
}

// Add the owl representation of the event to the owl document
void FSLGraspEvent::AddToOwlDoc(FOwlDoc* OutDoc)
{
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLGraspEvent::Context() const
{
	return FString("GraspEvent");
}
/* End ISLEvent interface */