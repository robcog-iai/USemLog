// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPreGraspPositioningEvent.h"
#include "SLOwlExperimentStatics.h"

// Constructor with initialization
FSLPreGraspPositioningEvent::FSLPreGraspPositioningEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLEntity& InManipulator, const FSLEntity& InItem) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), Manipulator(InManipulator), Item(InItem)
{
}

// Constructor initialization without end time
FSLPreGraspPositioningEvent::FSLPreGraspPositioningEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLEntity& InItem, const FSLEntity& InManipulator) :
	ISLEvent(InId, InStart), PairId(InPairId), Manipulator(InManipulator), Item(InItem)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLPreGraspPositioningEvent::ToOwlNode() const
{
	// Create the Reach event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "PreGraspPositioning");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Manipulator.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Item.Id));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLPreGraspPositioningEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
	EventsDoc->AddObjectIndividual(Manipulator.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Manipulator.Id, Manipulator.Class));
	EventsDoc->AddObjectIndividual(Item.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Item.Id, Item.Class));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLPreGraspPositioningEvent::Context() const
{
	return FString::Printf(TEXT("PreGrasp - %lld"), PairId);
}

// Get the tooltip data
FString FSLPreGraspPositioningEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Manipulator.Class, *Manipulator.Id, *Item.Class, *Item.Id, *Id);
}

// Get the data as string
FString FSLPreGraspPositioningEvent::ToString() const
{
	return FString::Printf(TEXT("Item:[%s] Manipulator:[%s] PairId:%lld"),
		*Manipulator.ToString(), *Item.ToString(), PairId);
}
/* End ISLEvent interface */