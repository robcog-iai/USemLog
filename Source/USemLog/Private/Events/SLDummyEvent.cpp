// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLDummyEvent.h"

// Default constructor
FSLDummyEvent::FSLDummyEvent()
{
	Start = 0.2f;
	End = 1231.f;
}


/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLDummyEvent::ToOwlNode(/*ESLEventsTemplate TemplateType*/) const
{
	return FSLOwlNode();
}

// Add the owl representation of the event to the owl document
void FSLDummyEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLDummyEvent::Context() const
{
	return FString("DummyEvent");
}
/* End ISLEvent interface */