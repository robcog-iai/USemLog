// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
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
	return EventIndividual;
}

// Send event through ROS
FString FSLSupportedByEvent::ToROSQuery() const
{
	// has_region, has_role, has_participant, has_time_interval, 
	// Action
	FString Query = FString::Printf(TEXT("Process = \'http://www.ease-crc.org/ont/SOMA.owl#PhysicsProcess_%s\',"), *Id);
	Query.Append("tell([");
	Query.Append("is_physical_process(Process),");
	Query.Append(FString::Printf(TEXT("has_type(EventType, soma:\'ForceInteraction\'),")));
	Query.Append("holds(Process, soma:\'isOccurrenceOf\', EventType),");
	Query.Append(FString::Printf(TEXT("holds(Process, soma:'hasEventBegin', %f),"), StartTime));
	Query.Append(FString::Printf(TEXT("holds(Process, soma:'hasEventEnd', %f)"), EndTime));
	Query.Append("]),");


	// Objects
	if (SupportingIndividual->IsLoaded() && SupportedIndividual->IsLoaded())
	{
		Query.Append(FString::Printf(TEXT("SupportingObj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *SupportingIndividual->GetClassValue(), *SupportingIndividual->GetIdValue()));
		Query.Append(FString::Printf(TEXT("SupportedObj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\',"), *SupportedIndividual->GetClassValue(), *SupportedIndividual->GetIdValue()));
		Query.Append("tell([");
		Query.Append("is_physical_object(SupportingObj),");
		Query.Append("is_physical_object(SupportedObj),");
		Query.Append(FString::Printf(TEXT("holds(SupportedObj, soma:\'isSupportedBy\', SupportingObj) during [%f,%f],"), StartTime, EndTime));
		Query.Append(FString::Printf(TEXT("holds(SupportingObj, soma:\'supports\', SupportedObj) during [%f,%f],"), StartTime, EndTime));
		Query.Append("has_participant(Process, SupportingObj),");
		Query.Append("has_participant(Process, SupportedObj)");
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
void FSLSupportedByEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint and object individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->AddTimepointIndividual(StartTime,
		FSLOwlExperimentStatics::CreateTimepointIndividual("log", StartTime));
	EventsDoc->AddTimepointIndividual(EndTime,
		FSLOwlExperimentStatics::CreateTimepointIndividual("log", EndTime));
	EventsDoc->AddObjectIndividual(SupportedIndividual,
		 FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportedIndividual->GetIdValue(), SupportedIndividual->GetClassValue()));
	EventsDoc->AddObjectIndividual(SupportingIndividual,
		FSLOwlExperimentStatics::CreateObjectIndividual("log", SupportingIndividual->GetIdValue(), SupportingIndividual->GetClassValue()));
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