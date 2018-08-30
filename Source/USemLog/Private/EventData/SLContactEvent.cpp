// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLContactEvent.h"
#include "OwlExperimentStatics.h"

// Default constructor
FSLContactEvent::FSLContactEvent() 
{
}

// Constructor with initialization
FSLContactEvent::FSLContactEvent(const FString& InId,
	const float InStart,
	const float InEnd,
	const uint32 InObj1Id,
	const FString& InObj1SemId,
	const FString& InObj1Class,
	const uint32 InObj2Id,
	const FString& InObj2SemId,
	const FString& InObj2Class) :
	ISLEvent(InId, InStart, InEnd),
	Obj1Id(InObj1Id),
	Obj1SemId(InObj1SemId),
	Obj1Class(InObj1Class),
	Obj2Id(InObj2Id),
	Obj2SemId(InObj2SemId),
	Obj2Class(InObj2Class)
{
}

// Constructor initialization without End
FSLContactEvent::FSLContactEvent(const FString& InId,
	const float InStart,
	const uint32 InObj1Id,
	const FString& InObj1SemId,
	const FString& InObj1Class,
	const uint32 InObj2Id,
	const FString& InObj2SemId,
	const FString& InObj2Class) :
	ISLEvent(InId, InStart),
	Obj1Id(InObj1Id),
	Obj1SemId(InObj1SemId),
	Obj1Class(InObj1Class),
	Obj2Id(InObj2Id),
	Obj2SemId(InObj2SemId),
	Obj2Class(InObj2Class)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FOwlNode FSLContactEvent::ToOwlNode() const
{
	// Create the contact event node
	FOwlNode EventIndividual = FOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "TouchingSituation");
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateInContactProperty("log", Obj1SemId));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateInContactProperty("log", Obj2SemId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLContactEvent::AddToOwlDoc(FOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FOwlExperiment* EventsDoc = static_cast<FOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(
		Start, FOwlExperimentStatics::CreateTimepointIndividual("log", Start));
	EventsDoc->AddTimepointIndividual(
		End, FOwlExperimentStatics::CreateTimepointIndividual("log", End));
	EventsDoc->AddObjectIndividual(Obj1Id,
		FOwlExperimentStatics::CreateObjectIndividual("log", Obj1SemId, Obj1Class));
	EventsDoc->AddObjectIndividual(Obj2Id,
		FOwlExperimentStatics::CreateObjectIndividual("log", Obj2SemId, Obj2Class));
	OutDoc->AddIndividual(ToOwlNode());
}
/* End ISLEvent interface */