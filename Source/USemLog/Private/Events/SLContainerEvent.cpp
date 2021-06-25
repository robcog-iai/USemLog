// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContainerEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLContainerEvent::FSLContainerEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	USLBaseIndividual* InManipulator, USLBaseIndividual* InOther, const FString& InType) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId),
	Manipulator(InManipulator), Individual(InOther), Type(InType)
{
}

// Constructor initialization without End with pair id
FSLContainerEvent::FSLContainerEvent(const FString& InId, const float InStart, const uint64 InPairId,
	USLBaseIndividual* InManipulator, USLBaseIndividual* InOther, const FString& InType) :
	ISLEvent(InId, InStart), PairId(InPairId),
	Manipulator(InManipulator), Individual(InOther), Type(InType)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLContainerEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "ContainerManipulation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Manipulator->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Individual->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateTypeProperty("knowrob", Type));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLContainerEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->RegisterTimepoint(StartTime);
	EventsDoc->RegisterTimepoint(EndTime);
	EventsDoc->RegisterObject(Manipulator);
	EventsDoc->RegisterObject(Individual);
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLContainerEvent::Context() const
{
	return FString::Printf(TEXT("ContainerManipulation - %lld"), PairId);
}

// Get the tooltip data
FString FSLContainerEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'Manipulator\',\'%s\',\'Id\',\'%s\',\'Other\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Manipulator->GetClassValue(), *Manipulator->GetIdValue(), *Individual->GetClassValue(), *Individual->GetIdValue(), *Id);
}

// Get the data as string
FString FSLContainerEvent::ToString() const
{
	return FString::Printf(TEXT("Manipulator:[%s] Other:[%s] PairId:%lld"),
		*Manipulator->GetInfo(), *Individual->GetInfo(), PairId);
}
/* End ISLEvent interface */