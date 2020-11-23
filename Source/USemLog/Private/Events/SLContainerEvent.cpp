// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
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
	return EventIndividual;
}

// Send event through ROS
FString FSLContainerEvent::ToROSQuery() const
{
	// has_region, has_role, has_participant, has_time_interval, 
	// Action
	FString Query = FString::Printf(TEXT("Action = \'http://www.ease-crc.org/ont/SOMA.owl#Manipulating_%s\',"), *Id);
	Query.Append("tell([");
	Query.Append("is_action(Action),");
	Query.Append(FString::Printf(TEXT("has_type(Task, soma:\'Manipulating\'),")));
	Query.Append("executes_task(Action, Task),");
	Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasEventBegin', %f),"), StartTime));
	Query.Append(FString::Printf(TEXT("holds(Action, soma:'hasEventEnd', %f)"), EndTime));
	Query.Append("]),");

	// Manipulator
	if (Manipulator->IsLoaded())
	{
		Query.Append(FString::Printf(TEXT("Manipulator = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *Manipulator->GetClassValue(), *Manipulator->GetIdValue()));
		Query.Append("tell([");
		Query.Append("holds(Action, soma:'isPerformedBy', Manipulator)");
		Query.Append("]),");
	}

	// Object Acted on
	if (Individual->IsLoaded())
	{
		Query.Append(FString::Printf(TEXT("ObjActedOn = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *Individual->GetClassValue(), *Individual->GetIdValue()));
		Query.Append("tell([");
		Query.Append("is_physical_object(ObjActedOn),");
		Query.Append(FString::Printf(TEXT("has_type(ContainerRole, soma:\'Container\'),")));
		Query.Append(FString::Printf(TEXT("has_role(CreatedObj, ContainerRole) during [%f,%f],"), StartTime, EndTime));
		Query.Append("has_participant(Action, ObjActedOn)");
		Query.Append("]),");
	}

	// Episode
	Query.Append("tell([");
	Query.Append("is_episode(Episode),");
	Query.Append("is_setting_for(Episode, Action)");
	Query.Append("]).");
	return Query;
}

// Add the owl representation of the event to the owl document
void FSLContainerEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(
		StartTime, FSLOwlExperimentStatics::CreateTimepointIndividual("log", StartTime));
	EventsDoc->AddTimepointIndividual(
		EndTime, FSLOwlExperimentStatics::CreateTimepointIndividual("log", EndTime));
	EventsDoc->AddObjectIndividual(Manipulator,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Manipulator->GetIdValue(), Manipulator->GetClassValue()));
	EventsDoc->AddObjectIndividual(Individual,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Individual->GetIdValue(), Individual->GetClassValue()));
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