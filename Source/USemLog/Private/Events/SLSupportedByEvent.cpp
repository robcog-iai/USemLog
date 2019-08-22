// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSupportedByEvent.h"
#include "SLOwlExperimentStatics.h"

// Constructor with initialization
FSLSupportedByEvent::FSLSupportedByEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLEntity& InSupportedItem, const FSLEntity& InSupportingItem) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), SupportedItem(InSupportedItem), SupportingItem(InSupportingItem)
{
}

// Constructor initialization without end time
FSLSupportedByEvent::FSLSupportedByEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLEntity& InSupportedItem, const FSLEntity& InSupportingItem) :
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
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportedProperty("log", SupportedItem.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportingProperty("log", SupportingItem.Id));
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
	EventsDoc->AddObjectIndividual(SupportedItem.Obj,
		 FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportedItem.Id, SupportedItem.Class));
	EventsDoc->AddObjectIndividual(SupportingItem.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportingItem.Id, SupportingItem.Class));
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
	return FString::Printf(TEXT("\'SupportedItem\',\'%s\',\'Id\',\'%s\',\'SupportingItem\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*SupportedItem.Class, *SupportedItem.Id, *SupportingItem.Class, *SupportingItem.Id, *Id);
}

// Get the data as string
FString FSLSupportedByEvent::ToString() const
{
	return FString::Printf(TEXT("SupportedItem:[%s] SupportingItem:[%s] PairId:%lld"),
		*SupportedItem.ToString(), *SupportingItem.ToString(), PairId);
}
/* End ISLEvent interface */