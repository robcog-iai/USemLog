// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEvent.h"
#include "SLOwlExperimentStatics.h"

// Constructor with initialization
FSLContactEvent::FSLContactEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLEntity& InItem1, const FSLEntity& InItem2) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), Item1(InItem1), Item2(InItem2)
{
}

// Constructor initialization without end time
FSLContactEvent::FSLContactEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLEntity& InItem1, const FSLEntity& InItem2) :
	ISLEvent(InId, InStart), PairId(InPairId), Item1(InItem1), Item2(InItem2)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLContactEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "TouchingSituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Item1.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Item2.Id));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLContactEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(
		Start, FSLOwlExperimentStatics::CreateTimepointIndividual("log", Start));
	EventsDoc->AddTimepointIndividual(
		End, FSLOwlExperimentStatics::CreateTimepointIndividual("log", End));
	EventsDoc->AddObjectIndividual(Item1.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Item1.Id, Item1.Class));
	EventsDoc->AddObjectIndividual(Item2.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Item2.Id, Item2.Class));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLContactEvent::Context() const
{
	return FString::Printf(TEXT("Contact - %lld"), PairId);
}

// Get the tooltip data
FString FSLContactEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Item1.Class, *Item1.Id, *Item2.Class, *Item2.Id, *Id);
}

// Get the data as string
FString FSLContactEvent::ToString() const
{
	return FString::Printf(TEXT("Item1:[%s] Item2:[%s] PairId:%lld"),
		*Item1.ToString(), *Item2.ToString(), PairId);
}
/* End ISLEvent interface */