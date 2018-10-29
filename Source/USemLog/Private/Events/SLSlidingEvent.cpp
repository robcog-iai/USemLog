// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSlidingEvent.h"
#include "SLOwlExperimentStatics.h"

	// Default constructor
FSLSlidingEvent::FSLSlidingEvent()
{
}

// Constructor with initialization
FSLSlidingEvent::FSLSlidingEvent(const FString& InId,
	const float InStart,
	const float InEnd,
	const uint32 InSupportedObjId,
	const FString& InSupportedObjSemId,
	const FString& InSupportedObjClass,
	const uint32 InSupportingObjId,
	const FString& InSupportingObjSemId,
	const FString& InSupportingObjClass) :
	ISLEvent(InId, InStart, InEnd),
	SupportedObjId(InSupportedObjId),
	SupportedObjSemId(InSupportedObjSemId),
	SupportedObjClass(InSupportedObjClass),
	SupportingObjId(InSupportingObjId),
	SupportingObjSemId(InSupportingObjSemId),
	SupportingObjClass(InSupportingObjClass)
{
}

// Constructor with initialization without end time
FSLSlidingEvent::FSLSlidingEvent(const FString& InId,
	const float InStart,
	const uint32 InSupportedObjId,
	const FString& InSupportedObjSemId,
	const FString& InSupportedObjClass,
	const uint32 InSupportingObjId,
	const FString& InSupportingObjSemId,
	const FString& InSupportingObjClass) :
	ISLEvent(InId, InStart),
	SupportedObjId(InSupportedObjId),
	SupportedObjSemId(InSupportedObjSemId),
	SupportedObjClass(InSupportedObjClass),
	SupportingObjId(InSupportingObjId),
	SupportingObjSemId(InSupportingObjSemId),
	SupportingObjClass(InSupportingObjClass)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLSlidingEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "SupportedBySituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportedProperty("log", SupportedObjSemId));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportingProperty("log", SupportingObjSemId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLSlidingEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint and object individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(
		Start, FSLOwlExperimentStatics::CreateTimepointIndividual("log", Start));
	EventsDoc->AddTimepointIndividual(
		End, FSLOwlExperimentStatics::CreateTimepointIndividual("log", End));
	EventsDoc->AddObjectIndividual(
		SupportedObjId, FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportedObjSemId, SupportedObjClass));
	EventsDoc->AddObjectIndividual(
		SupportingObjId, FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportingObjSemId, SupportingObjClass));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLSlidingEvent::Context() const
{
	return FString("SlidingEvent");
}

// Get the tooltip data
FString FSLSlidingEvent::Tooltip() const
{
	return FString("SlidingEvent");;
}
/* End ISLEvent interface */