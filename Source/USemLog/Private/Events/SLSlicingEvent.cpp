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

// Send event through ROS
FString FSLSlicingEvent::ToROSQuery() const
{
	// has_region, has_role, has_participant, has_time_interval, 
	// Action
	FString Query = FString::Printf(TEXT("Action = \'http://www.ease-crc.org/ont/SOMA.owl#Slicing_%s\',"), *Id);
	Query.Append("tell([");
	Query.Append("is_action(Action),");
	Query.Append(FString::Printf(TEXT("has_type(Task, soma:\'Slicing\'),")));
	Query.Append("executes_task(Action, Task),");
	Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasEventBegin', %f),"), Start));
	Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasEventEnd', %f),"), End));
	if (bTaskSuccessful) {
		Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasExecutionState', soma:'ExecutionState_Succeded')")));
	}
	else {
		Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasExecutionState', soma:'ExecutionState_Failed')")));
	}
	Query.Append("]),");

	// Tool
	if (DeviceUsed.IsSet()) {
		Query.Append(FString::Printf(TEXT("ObjUsed = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *DeviceUsed.Class, *DeviceUsed.Id));
		Query.Append("tell([");
		Query.Append(FString::Printf(TEXT("has_type(ObjUsed, soma:\'knife\'),")));
		Query.Append(FString::Printf(TEXT("has_type(ToolRole, soma:\'Tool\'),")));
		Query.Append(FString::Printf(TEXT("has_role(ObjUsed, ToolRole) during [%f,%f],"), Start, End));
		Query.Append("has_participant(Action, ObjUsed)");
		Query.Append("]),");
	}

	// Food
	if (ObjectActedOn.IsSet()) {
		Query.Append(FString::Printf(TEXT("AlteredObj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *ObjectActedOn.Class, *ObjectActedOn.Id));
		Query.Append("tell([");
		Query.Append("is_physical_object(AlteredObj),");
		Query.Append(FString::Printf(TEXT("has_type(AlteredRole, soma:\'AlteredObject\'),")));
		Query.Append(FString::Printf(TEXT("has_role(AlteredObj, AlteredRole) during [%f,%f],"), Start, End));
		Query.Append("has_participant(Action, AlteredObj)");
		Query.Append("]),");
	}

	// New piece
	if (OutputsCreated.IsSet()) {
		Query.Append(FString::Printf(TEXT("CreatedObj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *OutputsCreated.Class, *OutputsCreated.Id));
		Query.Append("tell([");
		Query.Append("is_physical_object(CreatedObj),");
		Query.Append(FString::Printf(TEXT("has_type(CreatedRole, soma:\'CreatedObject\'),")));
		Query.Append(FString::Printf(TEXT("has_role(CreatedObj, CreatedRole) during [%f,%f],"), Start, End));
		Query.Append("has_participant(Action, CreatedObj)");
		Query.Append("]),");
	}

	// Episode
	Query.Append("tell([");
	Query.Append("is_episode(Episode),");
	Query.Append("is_setting_for(Episode, Action)");
	Query.Append("]).");
	return Query;
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