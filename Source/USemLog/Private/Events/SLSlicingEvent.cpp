// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSlicingEvent.h"
#include "SLOwlExperimentStatics.h"

// Constructor with initialization
FSLSlicingEvent::FSLSlicingEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	const FSLEntity& InPerformedBy, const FSLEntity& InDeviceUsed, const FSLEntity& InObjectActedOn,
	const FSLEntity& InOutputsCreated, const bool bInTaskSuccessful) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId),
	PerformedBy(InPerformedBy), DeviceUsed(InDeviceUsed), ObjectActedOn(InObjectActedOn),
	OutputsCreated(InOutputsCreated), bTaskSuccessful(bInTaskSuccessful)
{
}

// Constructor initialization without endTime, endTaskSuccess and outputsCreated.
FSLSlicingEvent::FSLSlicingEvent(const FString& InId, const float InStart, const uint64 InPairId,
	const FSLEntity& InPerformedBy, const FSLEntity& InDeviceUsed, const FSLEntity& InObjectActedOn) :
	ISLEvent(InId, InStart), PairId(InPairId),
	PerformedBy(InPerformedBy), DeviceUsed(InDeviceUsed), ObjectActedOn(InObjectActedOn)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLSlicingEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "SlicingingSomething");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", Start));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", End));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", PerformedBy.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateDeviceUsedProperty("log", DeviceUsed.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", ObjectActedOn.Id));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateTaskSuccessProperty("log", bTaskSuccessful));
	if (bTaskSuccessful)
	{
		EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateOutputsCreatedProperty("log", OutputsCreated.Id));
	}
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLSlicingEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
	EventsDoc->AddObjectIndividual(PerformedBy.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", PerformedBy.Id, PerformedBy.Class));
	EventsDoc->AddObjectIndividual(DeviceUsed.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", DeviceUsed.Id, DeviceUsed.Class));
	EventsDoc->AddObjectIndividual(ObjectActedOn.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", ObjectActedOn.Id, ObjectActedOn.Class));
	if (bTaskSuccessful)
	{
	EventsDoc->AddObjectIndividual(OutputsCreated.Obj,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", OutputsCreated.Id, OutputsCreated.Class));
	}
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLSlicingEvent::Context() const
{
	return FString::Printf(TEXT("Slicing - %lld"), PairId);
}

// Get the tooltip data
FString FSLSlicingEvent::Tooltip() const
{
	if (bTaskSuccessful) {
		return FString::Printf(TEXT("\'PerformedBy\',\'%s\',\'Id\',\'%s\',  \
								 \'DeviceUsed\',\'%s\',\'Id\',\'%s\',  \
								 \'ObjectActedOn\',\'%s\',\'Id\',\'%s\',\
								 \'TaskSuccess True\', \
								 \'OutputsCreated\',\'%s\',\'Id\',\'%s\',\
								 \'Id\',\'%s\'"),
						*PerformedBy.Class, *PerformedBy.Id, 
						*DeviceUsed.Class, *DeviceUsed.Id,
						*ObjectActedOn.Class, *ObjectActedOn.Id, 
						*OutputsCreated.Class, *OutputsCreated.Id,
						*Id);
	}
	else {
		return FString::Printf(TEXT("\'PerformedBy\',\'%s\',\'Id\',\'%s\',  \
								 \'DeviceUsed\',\'%s\',\'Id\',\'%s\',  \
								 \'ObjectActedOn\',\'%s\',\'Id\',\'%s\',\
								 \'TaskSuccess False\', \
								 \'Id\',\'%s\'"),
			*PerformedBy.Class, *PerformedBy.Id,
			*DeviceUsed.Class, *DeviceUsed.Id,
			*ObjectActedOn.Class, *ObjectActedOn.Id,
			*Id);
	}
}

// Get the data as string
FString FSLSlicingEvent::ToString() const
{
	if (bTaskSuccessful) {
		return FString::Printf(TEXT("PerformedBy:[%s] DeviceUsed:[%s] ObjectActedOn:[%s] TaskSuccess:[True] OutputsCreated:[%s] PairId:%lld"),
			*PerformedBy.ToString(), *DeviceUsed.ToString(), *ObjectActedOn.ToString(), *OutputsCreated.ToString(), PairId);
	}
	else {
		return FString::Printf(TEXT("PerformedBy:[%s] DeviceUsed:[%s] ObjectActedOn:[%s] TaskSuccess:[False] PairId:%lld"),
			*PerformedBy.ToString(), *DeviceUsed.ToString(), *ObjectActedOn.ToString(), PairId);
	}
}
/* End ISLEvent interface */