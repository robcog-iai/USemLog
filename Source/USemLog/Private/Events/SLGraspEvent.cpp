// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLGraspEvent.h"
#include "SLOwlExperimentStatics.h"

// Constructor with initialization
FSLGraspEvent::FSLGraspEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLEntity& InManipulator, const FSLEntity& InOther, const FString& InGraspType) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId),
	Manipulator(InManipulator), Item(InOther), GraspType(InGraspType)
{
}

// Constructor initialization without End with pair id
FSLGraspEvent::FSLGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLEntity& InManipulator, const FSLEntity& InOther, const FString& InGraspType) :
	ISLEvent(InId, InStart), PairId(InPairId),
	Manipulator(InManipulator), Item(InOther), GraspType(InGraspType)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLGraspEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "GraspingSomething");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Manipulator.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Item.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateGraspTypeProperty("knowrob", GraspType));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLGraspEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
FString FSLGraspEvent::Context() const
{
	return FString::Printf(TEXT("Grasp - %lld"), PairId);
}

// Get the tooltip data
FString FSLGraspEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'Manipulator\',\'%s\',\'Id\',\'%s\',\'Other\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Manipulator.Class, *Manipulator.Id, *Item.Class, *Item.Id, *Id);
}

// Get the data as string
FString FSLGraspEvent::ToString() const
{
	return FString::Printf(TEXT("Manipulator:[%s] Other:[%s] PairId:%lld"),
		*Manipulator.ToString(), *Item.ToString(), PairId);
}
/* End ISLEvent interface */