// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSupportedByEvent.h"
#include "SLOwlExperimentStatics.h"

	// Default constructor
FSLSupportedByEvent::FSLSupportedByEvent()
{
}

// Constructor with initialization
FSLSupportedByEvent::FSLSupportedByEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLItem& InSupportedItem, const FSLItem& InSupportingItem) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), SupportedItem(InSupportedItem), SupportingItem(InSupportingItem)
{
}

// Constructor initialization without end time
FSLSupportedByEvent::FSLSupportedByEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLItem& InSupportedItem, const FSLItem& InSupportingItem) :
	ISLEvent(InId, InStart), PairId(InPairId), SupportedItem(InSupportedItem), SupportingItem(InSupportingItem)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLSupportedByEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "SupportedBySituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportedProperty("log", SupportedItem.SemId));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportingProperty("log", SupportingItem.SemId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLSupportedByEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint and object individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(Start,
		FSLOwlExperimentStatics::CreateTimepointIndividual("log", Start));
	EventsDoc->AddTimepointIndividual(End,
		FSLOwlExperimentStatics::CreateTimepointIndividual("log", End));
	EventsDoc->AddObjectIndividual(SupportedItem.Id,
		 FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportedItem.SemId, SupportedItem.Class));
	EventsDoc->AddObjectIndividual(SupportingItem.Id,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportingItem.SemId, SupportingItem.Class));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLSupportedByEvent::Context() const
{
	return FString::Printf(TEXT("SupportedBy - %lld"), PairId);
}

// Get the tooltip data
FString FSLSupportedByEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'SupportingItem\',\'%s\',\'Id\',\'%s\',\'SupportedItem\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*SupportingItem.Class, *SupportingItem.SemId, *SupportedItem.Class, *SupportedItem.SemId, *Id);
}
/* End ISLEvent interface */