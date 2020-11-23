// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLContactEvent::FSLContactEvent(const FString& InId, float InStart, float InEnd, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2) :
	ISLEvent(InId, InStart, InEnd),
	PairId(InPairId),
	Individual1(InIndividual1), 
	Individual2(InIndividual2)
{
}

// Constructor initialization without end time
FSLContactEvent::FSLContactEvent(const FString& InId, float InStart, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2) :
	ISLEvent(InId, InStart),
	PairId(InPairId),
	Individual1(InIndividual1),
	Individual2(InIndividual2)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLContactEvent::ToOwlNode() const
{
	// Create the contact event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "TouchingSituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Individual1->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Individual2->GetIdValue()));
	return EventIndividual;
}

// Send event through ROS
FString FSLContactEvent::ToROSQuery() const
{
	// has_region, has_role, has_participant, has_time_interval, 
	// Action
	FString Query = FString::Printf(TEXT("Process = \'http://www.ease-crc.org/ont/SOMA.owl#PhysicsProcess_%s\',"), *Id);
	Query.Append("tell([");
	Query.Append("is_physical_process(Process),");
	Query.Append(FString::Printf(TEXT("has_type(EventType, soma:\'Collision\'),")));
	Query.Append("holds(Process, soma:\'isOccurrenceOf\', EventType),");
	Query.Append(FString::Printf(TEXT("holds(Process, soma:'hasEventBegin', %f),"), StartTime));
	Query.Append(FString::Printf(TEXT("holds(Process, soma:'hasEventEnd', %f)"), EndTime));
	Query.Append("]),");


	// Objects
	if (Individual1->IsLoaded() && Individual2->IsLoaded())
	{
		Query.Append(FString::Printf(TEXT("Obj1 = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *Individual1->GetClassValue(), *Individual1->GetIdValue()));
		Query.Append(FString::Printf(TEXT("Obj2 = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *Individual2->GetClassValue(), *Individual2->GetIdValue()));
		Query.Append("tell([");
		Query.Append("is_physical_object(Obj1),");
		Query.Append("is_physical_object(Obj2),");
		Query.Append("has_participant(Process, Obj1),");
		Query.Append("has_participant(Process, Obj2)");
		Query.Append("]),");
	}

	// Episode
	Query.Append("tell([");
	Query.Append("is_episode(Episode),");
	Query.Append("is_setting_for(Episode, Process)");
	Query.Append("]).");
	return Query;
}

// Add the owl representation of the event to the owl document
void FSLContactEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
	EventsDoc->AddObjectIndividual(Individual1,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Individual1->GetIdValue(), Individual1->GetClassValue()));
	EventsDoc->AddObjectIndividual(Individual2,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", Individual2->GetIdValue(), Individual2->GetClassValue()));
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLContactEvent::Context() const
{
	return FString::Printf(TEXT("Contact - %lld"), PairId);
}

// Get the tooltip data
FString FSLContactEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Individual1->GetClassValue(), *Individual1->GetIdValue(), *Individual2->GetClassValue(), *Individual2->GetIdValue(), *Id);
}

// Get the data as string
FString FSLContactEvent::ToString() const
{
	return FString::Printf(TEXT("Individual1:[%s] Individual2:[%s] PairId:%lld"),
		*Individual1->GetInfo(), *Individual2->GetInfo(), PairId);
}
/* End ISLEvent interface */