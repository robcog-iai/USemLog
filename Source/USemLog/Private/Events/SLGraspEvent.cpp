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
	const uint32 InHandId, const FString& InHandSemId, const FString& InHandClass,
	const uint32 InOtherId, const FString& InOtherSemId, const FString& InOtherClass) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId),
	HandId(InHandId), HandSemId(InHandSemId), HandClass(InHandClass),
	OtherId(InOtherId), OtherSemId(InOtherSemId), OtherClass(InOtherClass)
{
}

// Constructor initialization without End with pair id
FSLGraspEvent::FSLGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const uint32 InHandId, const FString& InHandSemId, const FString& InHandClass,
	const uint32 InOtherId, const FString& InOtherSemId, const FString& InOtherClass) :
	ISLEvent(InId, InStart), PairId(InPairId),
	HandId(InHandId), HandSemId(InHandSemId), HandClass(InHandClass),
	OtherId(InOtherId), OtherSemId(InOtherSemId), OtherClass(InOtherClass)
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
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", HandSemId));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", OtherSemId));
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
	EventsDoc->AddObjectIndividual(HandId,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", HandSemId, HandClass));
	EventsDoc->AddObjectIndividual(OtherId,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", OtherSemId, OtherClass));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLGraspEvent::Context() const
{
	return FString::Printf(TEXT("GraspEvent - %lld"), PairId);
}

// Get the tooltip data
FString FSLGraspEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'Hand\',\'%s\',\'Id\',\'%s\',\'O\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*HandClass, *HandSemId, *OtherClass, *OtherSemId, *Id);
}
/* End ISLEvent interface */