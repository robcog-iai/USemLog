// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSupportedByEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLSupportedByEvent::FSLSupportedByEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	USLBaseIndividual* InSupportedIndividual, USLBaseIndividual* InSupportingIndividual) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), SupportedIndividual(InSupportedIndividual), SupportingIndividual(InSupportingIndividual)
{
}

// Constructor initialization without end time
FSLSupportedByEvent::FSLSupportedByEvent(const FString& InId, const float InStart, const uint64 InPairId,
	USLBaseIndividual* InSupportedIndividual, USLBaseIndividual* InSupportingIndividual) :
	ISLEvent(InId, InStart), PairId(InPairId), SupportedIndividual(InSupportedIndividual), SupportingIndividual(InSupportingIndividual)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLSupportedByEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "SupportedBySituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportedProperty("log", SupportedIndividual->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateIsSupportingProperty("log", SupportingIndividual->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
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
	EventsDoc->RegisterTimepoint(StartTime);
	EventsDoc->RegisterTimepoint(EndTime);
	EventsDoc->RegisterObject(SupportingIndividual);
	EventsDoc->RegisterObject(SupportedIndividual);
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
	return FString::Printf(TEXT("\'SupportedIndividual\',\'%s\',\'Id\',\'%s\',\'SupportingIndividual\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*SupportedIndividual->GetClassValue(), *SupportedIndividual->GetIdValue(), *SupportingIndividual->GetClassValue(), *SupportingIndividual->GetIdValue(), *Id);
}

// Get the data as string
FString FSLSupportedByEvent::ToString() const
{
	return FString::Printf(TEXT("SupportedIndividual:[%s] SupportingIndividual:[%s] PairId:%lld"),
		*SupportedIndividual->GetInfo(), *SupportingIndividual->GetInfo(), PairId);
}
/* End ISLEvent interface */