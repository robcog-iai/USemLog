// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSlicingEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLSlicingEvent::FSLSlicingEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	USLBaseIndividual* InPerformedBy, USLBaseIndividual* InDeviceUsed, USLBaseIndividual* InObjectActedOn,
	USLBaseIndividual* InOutputsCreated, const bool bInTaskSuccessful) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId),
	PerformedBy(InPerformedBy), DeviceUsed(InDeviceUsed), ObjectActedOn(InObjectActedOn),
	CreatedSlice(InOutputsCreated), bTaskSuccessful(bInTaskSuccessful)
{
}

// Constructor initialization without endTime, endTaskSuccess and outputsCreated.
FSLSlicingEvent::FSLSlicingEvent(const FString& InId, const float InStart, const uint64 InPairId,
	USLBaseIndividual* InPerformedBy, USLBaseIndividual* InDeviceUsed, USLBaseIndividual* InObjectActedOn) :
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
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", PerformedBy->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateDeviceUsedProperty("log", DeviceUsed->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", ObjectActedOn->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateTaskSuccessProperty("log", bTaskSuccessful));
	if (bTaskSuccessful)
	{
		EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateOutputsCreatedProperty("log", CreatedSlice->GetIdValue()));
	}
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
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
	EventsDoc->RegisterTimepoint(StartTime);
	EventsDoc->RegisterTimepoint(EndTime);
	EventsDoc->RegisterObject(PerformedBy);
	EventsDoc->RegisterObject(DeviceUsed);
	EventsDoc->RegisterObject(ObjectActedOn);
	EventsDoc->RegisterObject(CreatedSlice);
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
	Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasEventBegin', %f),"), StartTime));
	Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasEventEnd', %f),"), EndTime));
	if (bTaskSuccessful) 
	{
		Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasExecutionState', soma:'ExecutionState_Succeded')")));
	}
	else 
	{
		Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasExecutionState', soma:'ExecutionState_Failed')")));
	}
	Query.Append("]),");

	// Tool
	if (DeviceUsed->IsLoaded()) 
	{
		Query.Append(FString::Printf(TEXT("ObjUsed = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *DeviceUsed->GetClassValue(), *DeviceUsed->GetIdValue()));
		Query.Append("tell([");
		Query.Append(FString::Printf(TEXT("has_type(ObjUsed, soma:\'knife\'),")));
		Query.Append(FString::Printf(TEXT("has_type(ToolRole, soma:\'Tool\'),")));
		Query.Append(FString::Printf(TEXT("has_role(ObjUsed, ToolRole) during [%f,%f],"), StartTime, EndTime));
		Query.Append("has_participant(Action, ObjUsed)");
		Query.Append("]),");
	}

	// Food
	if (ObjectActedOn->IsLoaded()) 
	{
		Query.Append(FString::Printf(TEXT("AlteredObj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *ObjectActedOn->GetClassValue(), *ObjectActedOn->GetIdValue()));
		Query.Append("tell([");
		Query.Append("is_physical_object(AlteredObj),");
		Query.Append(FString::Printf(TEXT("has_type(AlteredRole, soma:\'AlteredObject\'),")));
		Query.Append(FString::Printf(TEXT("has_role(AlteredObj, AlteredRole) during [%f,%f],"), StartTime, EndTime));
		Query.Append("has_participant(Action, AlteredObj)");
		Query.Append("]),");
	}

	// New piece
	if (CreatedSlice->IsLoaded()) 
	{
		Query.Append(FString::Printf(TEXT("CreatedObj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *CreatedSlice->GetClassValue(), *CreatedSlice->GetIdValue()));
		Query.Append("tell([");
		Query.Append("is_physical_object(CreatedObj),");
		Query.Append(FString::Printf(TEXT("has_type(CreatedRole, soma:\'CreatedObject\'),")));
		Query.Append(FString::Printf(TEXT("has_role(CreatedObj, CreatedRole) during [%f,%f],"), StartTime, EndTime));
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
	if (bTaskSuccessful) 
	{
		return FString::Printf(TEXT("\'PerformedBy\',\'%s\',\'Id\',\'%s\',  \
								 \'DeviceUsed\',\'%s\',\'Id\',\'%s\',  \
								 \'ObjectActedOn\',\'%s\',\'Id\',\'%s\',\
								 \'TaskSuccess True\', \
								 \'OutputsCreated\',\'%s\',\'Id\',\'%s\',\
								 \'Id\',\'%s\'"),
						*PerformedBy->GetClassValue(), *PerformedBy->GetIdValue(), 
						*DeviceUsed->GetClassValue(), *DeviceUsed->GetIdValue(),
						*ObjectActedOn->GetClassValue(), *ObjectActedOn->GetIdValue(), 
						*CreatedSlice->GetClassValue(), *CreatedSlice->GetIdValue(),
						*Id);
	}
	else 
	{
		return FString::Printf(TEXT("\'PerformedBy\',\'%s\',\'Id\',\'%s\',  \
								 \'DeviceUsed\',\'%s\',\'Id\',\'%s\',  \
								 \'ObjectActedOn\',\'%s\',\'Id\',\'%s\',\
								 \'TaskSuccess False\', \
								 \'Id\',\'%s\'"),
			*PerformedBy->GetClassValue(), *PerformedBy->GetIdValue(),
			*DeviceUsed->GetClassValue(), *DeviceUsed->GetIdValue(),
			*ObjectActedOn->GetClassValue(), *ObjectActedOn->GetIdValue(),
			*Id);
	}
}

// Get the data as string
FString FSLSlicingEvent::ToString() const
{
	if (bTaskSuccessful) 
	{
		return FString::Printf(TEXT("PerformedBy:[%s] DeviceUsed:[%s] ObjectActedOn:[%s] TaskSuccess:[True] OutputsCreated:[%s] PairId:%lld"),
			*PerformedBy->GetInfo(), *DeviceUsed->GetInfo(), *ObjectActedOn->GetInfo(), *CreatedSlice->GetInfo(), PairId);
	}
	else 
	{
		return FString::Printf(TEXT("PerformedBy:[%s] DeviceUsed:[%s] ObjectActedOn:[%s] TaskSuccess:[False] PairId:%lld"),
			*PerformedBy->GetInfo(), *DeviceUsed->GetInfo(), *ObjectActedOn->GetInfo(), PairId);
	}
}
/* End ISLEvent interface */