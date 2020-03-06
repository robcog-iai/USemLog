// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPickUpEvent.h"
#include "SLOwlExperimentStatics.h"

// Constructor with initialization
FSLPickUpEvent::FSLPickUpEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLEntity& InManipulator, const FSLEntity& InItem) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), Manipulator(InManipulator), Item(InItem)
{
}

// Constructor initialization without end time
FSLPickUpEvent::FSLPickUpEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLEntity& InItem, const FSLEntity& InManipulator) :
	ISLEvent(InId, InStart), PairId(InPairId), Manipulator(InManipulator), Item(InItem)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLPickUpEvent::ToOwlNode() const
{
	// Create the Lift event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "PickUpSituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Manipulator.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Item.Id));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLPickUpEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
	EventsDoc->AddObjectIndividual(Item.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Item.Id, Manipulator.Class));
	EventsDoc->AddObjectIndividual(Manipulator.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Manipulator.Id, Manipulator.Class));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLPickUpEvent::Context() const
{
	return FString::Printf(TEXT("PickUp - %lld"), PairId);
}

// Get the tooltip data
FString FSLPickUpEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Item.Class, *Item.Id, *Manipulator.Class, *Manipulator.Id, *Id);
}

// Get the data as string
FString FSLPickUpEvent::ToString() const
{
	return FString::Printf(TEXT("Item:[%s] Manipulator:[%s] PairId:%lld"),
		*Item.ToString(), *Manipulator.ToString(), PairId);
}
/* End ISLEvent interface */