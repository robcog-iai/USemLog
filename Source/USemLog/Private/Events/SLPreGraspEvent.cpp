// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPreGraspEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLPreGraspEvent::FSLPreGraspEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	USLBaseIndividual* InManipulator, USLBaseIndividual* InIndividual) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), Manipulator(InManipulator), Individual(InIndividual)
{
}

// Constructor initialization without end time
FSLPreGraspEvent::FSLPreGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
	USLBaseIndividual* InIndividual, USLBaseIndividual* InManipulator) :
	ISLEvent(InId, InStart), PairId(InPairId), Manipulator(InManipulator), Individual(InIndividual)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLPreGraspEvent::ToOwlNode() const
{
	// Create the Reach event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "PreGrasp");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Manipulator->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Individual->GetIdValue()));
	return EventIndividual;
}

// Send event through ROS
FString FSLPreGraspEvent::ToROSQuery() const
{
	// has_region, has_role, has_participant, has_time_interval, 
	// Action
	FString Query = FString::Printf(TEXT("Action = \'http://www.ease-crc.org/ont/SOMA.owl#EndEffectorPositioning_%s\',"), *Id);
	Query.Append("tell([");
	Query.Append("is_action(Action),");
	Query.Append(FString::Printf(TEXT("has_type(Task, soma:\'EndEffectorPositioning\'),")));
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
void FSLPreGraspEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
FString FSLPreGraspEvent::Context() const
{
	return FString::Printf(TEXT("PreGrasp - %lld"), PairId);
}

// Get the tooltip data
FString FSLPreGraspEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Manipulator->GetClassValue(), *Manipulator->GetIdValue(), *Individual->GetClassValue(), *Individual->GetIdValue(), *Id);
}

// Get the data as string
FString FSLPreGraspEvent::ToString() const
{
	return FString::Printf(TEXT("Individual:[%s] Manipulator:[%s] PairId:%lld"),
		*Manipulator->GetInfo(), *Individual->GetInfo(), PairId);
}
/* End ISLEvent interface */