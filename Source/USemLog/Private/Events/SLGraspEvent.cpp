// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLGraspEvent.h"
#include "SLOwlExperimentStatics.h"

// Default constructor
FSLGraspEvent::FSLGraspEvent()
{
}

// Constructor with initialization
FSLGraspEvent::FSLGraspEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLItem& InHand, const FSLItem& InOther) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId),
	Hand(InHand), Other(InOther)
{
}

// Constructor initialization without End with pair id
FSLGraspEvent::FSLGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLItem& InHand, const FSLItem& InOther) :
	ISLEvent(InId, InStart), PairId(InPairId),
	Hand(InHand), Other(InOther)
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
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Hand.SemId));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Other.SemId));
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
	EventsDoc->AddObjectIndividual(Hand.Id,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Hand.SemId, Hand.Class));
	EventsDoc->AddObjectIndividual(Other.Id,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Other.SemId, Other.Class));
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
	return FString::Printf(TEXT("\'Hand\',\'%s\',\'Id\',\'%s\',\'Other\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Hand.Class, *Hand.SemId, *Other.Class, *Other.SemId, *Id);
}
/* End ISLEvent interface */