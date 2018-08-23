// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLContactEvent.h"
#include "OwlEventsStatics.h"

// Default constructor
FSLContactEvent::FSLContactEvent() 
{
}

// Constructor with initialization
FSLContactEvent::FSLContactEvent(const FString& InId,
	float InStart,
	float InEnd,
	const FString& InObj1Id,
	const FString& InObj2Id) :
	ISLEvent(InId, InStart, InEnd),
	Obj1Id(InObj1Id),
	Obj2Id(InObj2Id)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FOwlNode FSLContactEvent::ToOwlNode() const
{
	// Create the contact event node
	FOwlNode EventIndividual = FOwlEventsStatics::CreateEventIndividual(
		"log", Id, "TouchingSituation");
	EventIndividual.AddChildNode(FOwlEventsStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FOwlEventsStatics::CreateEndTimeProperty("log", End));	
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLContactEvent::AddToOwlDoc(FOwlDoc* OutDoc)
{
	OutDoc->AddIndividual(ToOwlNode());
}
/* End ISLEvent interface */