// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLSupportedByEvent.h"
#include "OwlExperimentStatics.h"

/* Begin ISLEvent interface */
// Get an owl representation of the event
FOwlNode FSLSupportedByEvent::ToOwlNode() const
{
	// Create the contact event node
	FOwlNode EventIndividual = FOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "SupportedBySituation");
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateIsSupportedProperty("log", Obj1Id));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateSupportsProperty("log", Obj2Id));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLSupportedByEvent::AddToOwlDoc(FOwlDoc* OutDoc)
{
	// Add timepoint and object individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FOwlExperiment* EventsDoc = static_cast<FOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(
		Start, FOwlExperimentStatics::CreateTimepointIndividual("log", Start));
	EventsDoc->AddTimepointIndividual(
		End, FOwlExperimentStatics::CreateTimepointIndividual("log", End));
	EventsDoc->AddObjectIndividual(Obj1UniqueId,
		FOwlExperimentStatics::CreateObjectIndividual("log", Obj1Id, Obj1Class));
	EventsDoc->AddObjectIndividual(Obj2UniqueId,
		FOwlExperimentStatics::CreateObjectIndividual("log", Obj2Id, Obj2Class));
	OutDoc->AddIndividual(ToOwlNode());
}
/* End ISLEvent interface */