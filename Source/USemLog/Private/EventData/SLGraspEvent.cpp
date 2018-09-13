// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLGraspEvent.h"
#include "OwlExperimentStatics.h"

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
FOwlNode FSLGraspEvent::ToOwlNode() const
{
	// Create the contact event node
	FOwlNode EventIndividual = FOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "GraspingSomething");
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreatePerformedByProperty("log", HandSemId));
	EventIndividual.AddChildNode(FOwlExperimentStatics::CreateObjectActedOnProperty("log", OtherSemId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLGraspEvent::AddToOwlDoc(FOwlDoc* OutDoc)
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
	EventsDoc->AddObjectIndividual(HandId,
		FOwlExperimentStatics::CreateObjectIndividual("log", HandSemId, HandClass));
	EventsDoc->AddObjectIndividual(OtherId,
		FOwlExperimentStatics::CreateObjectIndividual("log", OtherSemId, OtherClass));
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