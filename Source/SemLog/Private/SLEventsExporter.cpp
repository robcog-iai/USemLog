// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"
#include "SLUtils.h"
#include "SLEventsExporter.h"

// Constructor
FSLEventsExporter::FSLEventsExporter(
	const FString UniqueTag,
	const TMap<AActor*, FString>& ActorToUniqueName,
	const TMap<AActor*, FString>& ActorToClassType,
	const float Timestamp)
{
	// Set episode unique tag
	EpisodeUniqueTag = UniqueTag;

	// Set the map references to the member maps
	EvActorToUniqueName = ActorToUniqueName;
	EvActorToClassType = ActorToClassType;

	// Init metadata
	Metadata = new SLEventStruct("&log;",
		"UnrealExperiment_" + EpisodeUniqueTag, Timestamp);
	// Add class property
	Metadata->Properties.Add(FSLUtils::SLOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;UnrealExperiment"));
	// Add experiment unique name tag
	Metadata->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:experiment", "rdf:datatype", "&xsd;string",
		FSLUtils::FStringToChar(EpisodeUniqueTag)));
	// Add startTime property
	Metadata->Properties.Add(
		FSLUtils::SLOwlTriple("knowrob:startTime", "rdf:resource",
			FSLUtils::FStringToChar("&log;" +
				FSLEventsExporter::AddTimestamp(Timestamp))));

	UE_LOG(SemLogEvent, Log, TEXT(" ** FSLEventsExporter Constr !"));
}

// Write events to file
void FSLEventsExporter::WriteEvents(const FString Path, const float Timestamp, bool bWriteTimelines)
{
	// End all opened events
	FSLEventsExporter::TerminateEvents(Timestamp);
	// Set metadata as finished
	Metadata->End = Timestamp;
	// Add endTime property
	Metadata->Properties.Add(
		FSLUtils::SLOwlTriple("knowrob:endTime", "rdf:resource",
			FSLUtils::FStringToChar("&log;" + 
				FSLEventsExporter::AddTimestamp(Timestamp))));

	///////// DOC
	rapidxml::xml_document<>* EventsDoc = new rapidxml::xml_document<>();

	///////// TYPE DECLARATION
	rapidxml::xml_node<> *DeclarationNode = EventsDoc->allocate_node(rapidxml::node_declaration);
	// Create attibutes
	FSLUtils::AddNodeAttribute(EventsDoc, DeclarationNode, "version", "1.0");
	FSLUtils::AddNodeAttribute(EventsDoc, DeclarationNode, "encoding", "utf-8");
	// Add node to document
	EventsDoc->append_node(DeclarationNode);

	///////// DOCTYPE
	const char* doctype = "rdf:RDF[ \n"
		"\t<!ENTITY rdf \"http://www.w3.org/1999/02/22-rdf-syntax-ns\">\n"
		"\t<!ENTITY rdfs \"http://www.w3.org/2000/01/rdf-schema\">\n"
		"\t<!ENTITY owl \"http://www.w3.org/2002/07/owl\">\n"
		"\t<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">\n"
		"\t<!ENTITY knowrob \"http://knowrob.org/kb/knowrob.owl#\">\n"
		"\t<!ENTITY knowrob_u \"http://knowrob.org/kb/knowrob_u.owl#\">\n"
		"\t<!ENTITY log \"http://knowrob.org/kb/unreal_log.owl#\">\n"
		"\t<!ENTITY u-map \"http://knowrob.org/kb/u_map.owl#\">\n"
		"]";
	// Create doctype node
	rapidxml::xml_node<> *DoctypeNode = EventsDoc->allocate_node(rapidxml::node_doctype, "", doctype);
	// Add node to document
	EventsDoc->append_node(DoctypeNode);

	///////// RDF NODE
	rapidxml::xml_node<>* RDFNode = EventsDoc->allocate_node(rapidxml::node_element, "rdf:RDF", "");
	// Add attributes
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:swrl", "http://www.w3.org/2003/11/swrl#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:owl", "http://www.w3.org/2002/07/owl#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:knowrob", "http://knowrob.org/kb/knowrob.owl#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xmlns:u-map", "http://knowrob.org/kb/u_map.owl#");
	FSLUtils::AddNodeAttribute(EventsDoc, RDFNode,
		"xml:base", "http://knowrob.org/kb/u_map.owl#");

	///////// ONTOLOGY IMPORT
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Ontologies");
	// Create entity node with ontologies as properties
	TArray<FSLUtils::SLOwlTriple> Ontologies;
	Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_robcog/owl/knowrob_u.owl"));
	FSLUtils::AddNodeEntityWithProperties(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Ontology", "rdf:about", "http://knowrob.org/kb/unreal_log.owl"),
		Ontologies);

	///////// GENERAL DEFINITIONS
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Property Definitions");
	// Object property definitions
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;taskContext"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;taskSuccess"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;startTime"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;endTime"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;experiment"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob_u;inContact"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob_u;semanticMap"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob_u;rating"));
	// Class definitions
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Class Definitions");
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob;GraspingSomething"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;UnrealExperiment"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;TouchingSituation"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;KitchenEpisode"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;ParticleTranslation"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateClosed"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateHalfClosed"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateOpened"));
	FSLUtils::AddNodeTriple(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateHalfOpened"));

	///////// EVENT INDIVIDUALS
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Event Individuals");
	// Add event individuals to RDF node
	for (const auto FinishedEventItr : FinishedEvents)
	{
		FSLUtils::AddNodeEntityWithProperties(EventsDoc, RDFNode,
			FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
				FSLUtils::FStringToChar(FinishedEventItr->Ns + FinishedEventItr->Name)),
			FinishedEventItr->Properties);
	}

	///////// OBJECT INDIVIDUALS
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Object Individuals");


	// Add event individuals to RDF node
	for (const auto ObjIndividualItr : ObjectIndividuals)
	{		
		// Check that both unique name and class is available
		if ((EvActorToUniqueName.Contains(ObjIndividualItr)) &&
			(EvActorToClassType.Contains(ObjIndividualItr)))
		{
			FSLUtils::AddNodeEntityWithProperty(EventsDoc, RDFNode,
				FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
					FSLUtils::FStringToChar("&log;" + EvActorToUniqueName[ObjIndividualItr])),
				FSLUtils::SLOwlTriple("rdf:type", "rdf:resource",
					FSLUtils::FStringToChar("&knowrob;" + EvActorToClassType[ObjIndividualItr])));
		}
		else
		{
			UE_LOG(SemLogEvent, Error, TEXT("%s 's unique name is not set! Writing object individual skipped!"), *ObjIndividualItr->GetName());
		}
	}

	///////// TIMEPOINT INDIVIDUALS
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Timepoint Individuals");
	// Add time individuals to RDF node
	for (const auto TimepointIter : TimepointIndividuals)
	{
		FSLUtils::AddNodeEntityWithProperty(EventsDoc, RDFNode,
			FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
				FSLUtils::FStringToChar("&log;" + TimepointIter)),
			FSLUtils::SLOwlTriple("rdf:type", "rdf:resource", "&knowrob;TimePoint"));
	};

	///////// METADATA INDIVIDUAL
	FSLUtils::AddNodeComment(EventsDoc, RDFNode, "Metadata Individual");
	// Add metadata to RDF node
	FSLUtils::AddNodeEntityWithProperties(EventsDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
			FSLUtils::FStringToChar(Metadata->Ns + Metadata->Name)),
		Metadata->Properties);
	

	///////// ADD RDF TO OWL DOC
	EventsDoc->append_node(RDFNode);

	// Create string
	std::string RapidXmlString;
	rapidxml::print(std::back_inserter(RapidXmlString), *EventsDoc, 0 /*rapidxml::print_no_indenting*/);
	FString OwlString = UTF8_TO_TCHAR(RapidXmlString.c_str());

	// Write string to file
	const FString FilePath = Path + "/EventData_" + EpisodeUniqueTag + ".owl";
	FFileHelper::SaveStringToFile(OwlString, *FilePath);

	// Write the events as timelines
	if (bWriteTimelines)
	{
		FSLEventsExporter::WriteTimelines(Path + "/Timelines_" + EpisodeUniqueTag + ".html");
	}
}

// Add beginning of grasping event
void FSLEventsExporter::BeginGraspingEvent(
	AActor* Self, AActor* Other, const float Timestamp)
{
	const FString HandName = Self->GetName();
	const FString GraspedActorName = Other->GetName();

	// Skip saving the event if one of the actor is not registered with unique name
	if (!(EvActorToUniqueName.Contains(Self) && EvActorToUniqueName.Contains(Other)))
	{
		UE_LOG(SemLogEvent, Error, TEXT(" %s or %s's unique name is not set! Begin grasp event skipped!"), *HandName, *GraspedActorName);
		return;
	}
	// Get unique name of the hand and object
	const FString HandUniqueName = EvActorToUniqueName[Self];
	const FString GraspedActorUniqueName = EvActorToUniqueName[Other];
	
	// Add hand and object to the object individuals array
	ObjectIndividuals.AddUnique(Self);
	ObjectIndividuals.AddUnique(Other);

	UE_LOG(SemLogEvent, Warning, TEXT("Begin Grasp[%s --> %s]"), *HandUniqueName, *GraspedActorUniqueName);
	
	// Create unique name of the event
	const FString EventUniqueName = "GraspingSomething_" + FSLUtils::GenerateRandomFString(4);
	// Init grasp event
	SLEventStruct* GraspEvent = new SLEventStruct("&log;", EventUniqueName, Timestamp);
	// Add class property
	GraspEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;GraspingSomething"));
	// Add taskContext
	GraspEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:taskContext", "rdf:datatype", "&xsd;string", 
		FSLUtils::FStringToChar("Grasp-" + HandUniqueName + "-" + GraspedActorUniqueName)));
	// Add startTime
	GraspEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:startTime", "rdf:resource", 
		FSLUtils::FStringToChar("&log;" + 
			FSLEventsExporter::AddTimestamp(Timestamp))));
	// Add objectActedOn
	GraspEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:objectActedOn", "rdf:resource", 
		FSLUtils::FStringToChar("&log;" + GraspedActorUniqueName)));
	// Add objectActedOn
	GraspEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:performedBy", "rdf:resource",
		FSLUtils::FStringToChar("&log;" + HandUniqueName)));

	// Add events to the map
	NameToOpenedEventsMap.Add("Grasp" + HandName + GraspedActorName, GraspEvent);
}

// Add ending of grasping event
void FSLEventsExporter::EndGraspingEvent(
	AActor* Self, AActor* Other, const float Timestamp)
{
	const FString HandName = Self->GetName();
	const FString GraspedActorName = Other->GetName();

	UE_LOG(SemLogEvent, Warning, TEXT("End Grasp[%s --> %s]"), *HandName, *GraspedActorName);

	// Check if grasp is started
	if (NameToOpenedEventsMap.Contains("Grasp" + HandName + GraspedActorName))
	{
		// Get and remove the event from the opened events map
		SLEventStruct* CurrGraspEv;
		NameToOpenedEventsMap.RemoveAndCopyValue("Grasp" + HandName + GraspedActorName, CurrGraspEv);

		// Add finishing time
		CurrGraspEv->End = Timestamp;

		// Add endTime property
		CurrGraspEv->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:endTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" + 
					FSLEventsExporter::AddTimestamp(Timestamp))));

		// Add as subAction property to Metadata
		Metadata->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:subAction", "rdf:resource",
				FSLUtils::FStringToChar(CurrGraspEv->Ns + CurrGraspEv->Name)));

		// Add event to the finished events array
		FinishedEvents.Add(CurrGraspEv);
	}
	else
	{
		UE_LOG(SemLogEvent, Error, TEXT(" Trying to end grasp which did not start: %s !"), *FString("Grasp" + HandName + GraspedActorName));
	}
}

// Add beginning of touching event
void FSLEventsExporter::BeginTouchingEvent(
	AActor* TriggerParent, AActor* OtherActor, const float Timestamp)
{
	const FString TriggerParentName = TriggerParent->GetName();
	const FString OtherActorName = OtherActor->GetName();

	// Skip saving the event if one of the actor is not registered with unique name
	if (!(EvActorToUniqueName.Contains(TriggerParent) && EvActorToUniqueName.Contains(OtherActor)))
	{
		UE_LOG(SemLogEvent, Error, TEXT(" %s or %s's unique name is not set! Begin touch event skipped!"), *TriggerParent->GetName(), *OtherActor->GetName());
		return;
	}
	// Get unique names of the objects in contact
	const FString TriggerUniqueName = EvActorToUniqueName[TriggerParent];
	const FString OtherActorUniqueName = EvActorToUniqueName[OtherActor];

	// Add objects to the object individuals array
	ObjectIndividuals.AddUnique(TriggerParent);	
	ObjectIndividuals.AddUnique(OtherActor);

	UE_LOG(SemLogEvent, Warning, TEXT("Begin Contact[%s <--> %s]"),	*TriggerParentName, *OtherActorName);

	// Create unique name of the event
	const FString EventUniqueName = "TouchingSituation_" + FSLUtils::GenerateRandomFString(4);
	// Init contact event
	SLEventStruct* ContactEvent = new SLEventStruct("&log;", EventUniqueName, Timestamp);
	// Add class property
	ContactEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob_u;TouchingSituation"));
	// Add taskContext
	ContactEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:taskContext", "rdf:datatype", "&xsd;string",
		FSLUtils::FStringToChar("Contact-" + TriggerUniqueName + "-" + OtherActorUniqueName)));
	// Add startTime
	ContactEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob:startTime", "rdf:resource",
		FSLUtils::FStringToChar("&log;" +
			FSLEventsExporter::AddTimestamp(Timestamp))));
	// Add in contact 1
	ContactEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob_u:inContact", "rdf:resource",
		FSLUtils::FStringToChar("&log;" + TriggerUniqueName)));
	// Add in contact 2
	ContactEvent->Properties.Add(FSLUtils::SLOwlTriple(
		"knowrob_u:inContact", "rdf:resource",
		FSLUtils::FStringToChar("&log;" + OtherActorUniqueName)));

	// Add events to the map
	NameToOpenedEventsMap.Add("Contact" + TriggerParentName + OtherActorName, ContactEvent);
}

// Add end of touching event
void FSLEventsExporter::EndTouchingEvent(
	AActor* TriggerParent, AActor* OtherActor, const float Timestamp)
{
	const FString TriggerParentName = TriggerParent->GetName();
	const FString OtherActorName = OtherActor->GetName();

	UE_LOG(SemLogEvent, Warning,
		TEXT("End Contact[%s <--> %s]"), *TriggerParentName, *OtherActorName);

	// Check if grasp is started
	if (NameToOpenedEventsMap.Contains("Contact" + TriggerParent->GetName() + OtherActor->GetName()))
	{
		// Get and remove the event from the opened events map
		SLEventStruct* CurrContactEv;
		NameToOpenedEventsMap.RemoveAndCopyValue("Contact" + TriggerParentName + OtherActorName,
			CurrContactEv);

		// Add finishing time
		CurrContactEv->End = Timestamp;

		// Add endTime property
		CurrContactEv->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:endTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" +
					FSLEventsExporter::AddTimestamp(Timestamp))));

		// Add as subAction property to Metadata
		Metadata->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:subAction", "rdf:resource",
				FSLUtils::FStringToChar(CurrContactEv->Ns + CurrContactEv->Name)));

		// Add event to the finished events array
		FinishedEvents.Add(CurrContactEv);
	}
	else
	{
		UE_LOG(SemLogEvent, Error, TEXT(" Trying to end a contact event which did not start: %s !"),
			*FString("Contact" + TriggerParent->GetName() + OtherActor->GetName()));
	}
}

// Add furniture state event
void FSLEventsExporter::FurnitureStateEvent(
	AActor* Furniture, const FString State, const float Timestamp)
{
	const FString FurnitureName = Furniture->GetName();

	// Skip saving the event if the furniture is not registered with a unique name
	if (!(EvActorToUniqueName.Contains(Furniture)))
	{
		UE_LOG(SemLogEvent, Error, TEXT(" %s's unique name is not set! Furniture state event skipped!"), *Furniture->GetName());
		return;
	}
	// Get unique names of the objects in contact
	const FString FurnitureUniqueName = EvActorToUniqueName[Furniture];
	
	// Check if the furniture has any opened events
	SLEventStruct* FurnitureEvent = NameToOpenedEventsMap.FindRef("FurnitureState" + FurnitureUniqueName);

	if (!FurnitureEvent)
	{
		// Create first event
		UE_LOG(SemLogEvent, Warning, TEXT("*Init*FurnitureState[%s <--> %s]"), *FurnitureName, *State);
		// Create unique name of the event
		const FString EventUniqueName = "FurnitureState" + State + "_" + FSLUtils::GenerateRandomFString(4);
		// Create the event
		FurnitureEvent = new SLEventStruct("&log;", EventUniqueName, Timestamp);
		// Add class property
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"rdf:type", "rdf:resource", 
			FSLUtils::FStringToChar("&knowrob_u;FurnitureState" + State)));
		// Add taskContext
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"knowrob:taskContext", "rdf:datatype", "&xsd;string",
			FSLUtils::FStringToChar("FurnitureState" + State + "-" + FurnitureUniqueName)));
		// Add startTime
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"knowrob:startTime", "rdf:resource",
			FSLUtils::FStringToChar("&log;" +
				FSLEventsExporter::AddTimestamp(Timestamp))));
		// Add object acted on
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"knowrob:objectActedOn", "rdf:resource",
			FSLUtils::FStringToChar("&log;" + FurnitureUniqueName)));

		// Add init furniture event to map
		NameToOpenedEventsMap.Add("FurnitureState" + FurnitureUniqueName, FurnitureEvent);
	}
	else
	{
		// Terminate previous furniture event
		UE_LOG(SemLogEvent, Warning, TEXT("*Update*FurnitureState[%s <--> %s]"), *FurnitureName, *State);
		// Add finishing time
		FurnitureEvent->End = Timestamp;

		// Add endTime property
		FurnitureEvent->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:endTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" +
					FSLEventsExporter::AddTimestamp(Timestamp))));

		// Add as subAction property to Metadata
		Metadata->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:subAction", "rdf:resource",
				FSLUtils::FStringToChar(FurnitureEvent->Ns + FurnitureEvent->Name)));

		// Add event to the finished events array
		FinishedEvents.Add(FurnitureEvent);

		// Create unique name of the event
		const FString EventUniqueName = "FurnitureState" + State + "_" + FSLUtils::GenerateRandomFString(4);
		// Create the event
		FurnitureEvent = new SLEventStruct("&log;", EventUniqueName, Timestamp);
		// Add class property
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"rdf:type", "rdf:resource",
			FSLUtils::FStringToChar("&knowrob_u;FurnitureState" + State)));
		// Add taskContext
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"knowrob:taskContext", "rdf:datatype", "&xsd;string",
			FSLUtils::FStringToChar("FurnitureState" + State + "-" + FurnitureUniqueName)));
		// Add startTime
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"knowrob:startTime", "rdf:resource",
			FSLUtils::FStringToChar("&log;" +
				FSLEventsExporter::AddTimestamp(Timestamp))));
		// Add object acted on
		FurnitureEvent->Properties.Add(FSLUtils::SLOwlTriple(
			"knowrob:objectActedOn", "rdf:resource",
			FSLUtils::FStringToChar("&log;" + FurnitureUniqueName)));

		// Add new opened event to the map
		NameToOpenedEventsMap.Add("FurnitureState" + FurnitureUniqueName, FurnitureEvent);
	}
}

// Terminate all dangling events
void FSLEventsExporter::TerminateEvents(const float Timestamp)
{
	// Iterate all opened events
	for (const auto NameToEvItr : NameToOpenedEventsMap)
	{
		// Add finishing time
		NameToEvItr.Value->End = Timestamp;

		// Add endTime property
		NameToEvItr.Value->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:endTime", "rdf:resource",
				FSLUtils::FStringToChar("&log;" + 
					FSLEventsExporter::AddTimestamp(Timestamp))));

		// Add as subAction property to Metadata
		Metadata->Properties.Add(
			FSLUtils::SLOwlTriple("knowrob:subAction", "rdf:resource",
				FSLUtils::FStringToChar(NameToEvItr.Value->Ns + NameToEvItr.Value->Name)));

		// Add event to the finished events array
		FinishedEvents.Add(NameToEvItr.Value);

		UE_LOG(SemLogEvent, Warning,
			TEXT("Terminate [%s]"), *NameToEvItr.Key);
	}

	// Empty the open events map
	NameToOpenedEventsMap.Empty();
}

// Write events as timelines
void FSLEventsExporter::WriteTimelines(const FString FilePath)
{
	FString TimelineStr = "<html>\n"
		"<script type=\"text/javascript\" src=\"https://www.google.com/jsapi?autoload={'modules':[{'name':'visualization',\n"
		"\t'version':'1','packages':['timeline']}]}\"></script>\n"
		"<script type=\"text/javascript\">\n"
		"google.setOnLoadCallback(drawChart);\n"
		"\n"
		"function drawChart() {\n"
		"  var container = document.getElementById('EventsTimelines');\n\n"
		"  var chart = new google.visualization.Timeline(container);\n\n"
		"  var dataTable = new google.visualization.DataTable();\n\n"

		"  dataTable.addColumn({ type: 'string', id: 'Event' });\n"
		"  dataTable.addColumn({ type: 'number', id: 'Start' });\n"
		"  dataTable.addColumn({ type: 'number', id: 'End' });\n\n"
		"  dataTable.addRows([\n";

	// Add events to the timelines
	for (const auto FinishedEventItr : FinishedEvents)
	{
		TimelineStr.Append("    [ '" + FinishedEventItr->Name + "', "
			+ FString::SanitizeFloat(FinishedEventItr->Start) + ", "
			+ FString::SanitizeFloat(FinishedEventItr->End) + "],\n");
	}


	TimelineStr.Append(
		"  ]); \n\n"
		"  chart.draw(dataTable);\n"
		"}\n"
		"</script>\n"
		"<div id=\"sim_timeline_ex\" style=\"width: 1300px; height: 900px;\"></div>\n\n"
		"</html>"
	);

	// Write timeline string to file
	FFileHelper::SaveStringToFile(TimelineStr, *FilePath);
}

// Add timepoint to array, and return Knowrob specific timestamp
inline const FString FSLEventsExporter::AddTimestamp(const float Timestamp)
{
	// KnowRob Timepoint
	const FString TimepointStr = "timepoint_" + FString::SanitizeFloat(Timestamp);
	// Add to timepoints array
	TimepointIndividuals.AddUnique(TimepointStr);
	// Return string ts
	return TimepointStr;
}
