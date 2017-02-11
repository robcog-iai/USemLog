// Fill out your copyright notice in the Description page of Project Settings.

#include "USemLogPrivatePCH.h"
#include "SLEventsExporter.h"

// Constructor
FSLEventsExporter::FSLEventsExporter(
	const FString UniqueTag,
	const TMap<AActor*, FString>& ActorToUniqueName,
	const TMap<AActor*, TArray<TPair<FString, FString>>>& ActorToSemLogInfo,
	const float Timestamp) :
	EpisodeUniqueTag(UniqueTag),
	ActToUniqueName(ActorToUniqueName)
{
	// Disable listening to events
	bListenToEvents = false;

	// Set the map references to the member maps
	ActToClassType = FSLUtils::GetMapOfSemLogInfoToActorToClass(ActorToSemLogInfo, "Class");

	// Init metadata
	FSLEventsExporter::InitMetadata(Timestamp);
}

// Write events to file
void FSLEventsExporter::WriteEvents(const FString Path, const float Timestamp, bool bWriteTimelines)
{
	// End all opened events
	FSLEventsExporter::TerminateEvents(Timestamp);
	
	// Finish metadata
	FSLEventsExporter::FinishMetadata(Timestamp);
	
	// Create the OWL document
	rapidxml::xml_document<>* EventsDoc = new rapidxml::xml_document<>();
	
	// Add document declarations
	FSLEventsExporter::AddDocumentDeclarations(EventsDoc);
	
	// Create parent RDF node
	rapidxml::xml_node<>* RDFNode = EventsDoc->allocate_node(rapidxml::node_element, "rdf:RDF", "");
	
	// Add RDF node attributes
	FSLEventsExporter::AddRDFNodeAttributes(EventsDoc, RDFNode);
	
	// Add ontology imports
	AddNodeComment(EventsDoc, RDFNode, "Ontologies");
	// Create entity node with ontologies as properties
	FSLOwlObjectIndividual* OntologiesF = new FSLOwlObjectIndividual(
		"owl:Ontology", "rdf:about", "http://knowrob.org/kb/unreal_log.owl");
	OntologiesF->AddProperty(FSLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	OntologiesF->AddProperty(FSLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob_u.owl"));
	OntologiesF->AddProperty(FSLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/sherpa.owl"));
	OntologiesF->AddToDocument(EventsDoc, RDFNode);
	
	// GENERAL DEFINITIONS
	// Object property definitions
	AddNodeComment(EventsDoc, RDFNode, "Property Definitions");
	FSLOwlDefinitions* PropertyDefinitions = new FSLOwlDefinitions("owl:ObjectProperty", "rdf:about");
	PropertyDefinitions->AddObject("&knowrob;taskContext");
	PropertyDefinitions->AddObject("&knowrob;taskSuccess");
	PropertyDefinitions->AddObject("&knowrob;startTime");
	PropertyDefinitions->AddObject("&knowrob;endTime");
	PropertyDefinitions->AddObject("&knowrob;experiment");
	PropertyDefinitions->AddObject("&knowrob_u;inContact");
	PropertyDefinitions->AddObject("&knowrob_u;semanticMap");
	PropertyDefinitions->AddObject("&knowrob_u;rating");
	PropertyDefinitions->AddToDocument(EventsDoc, RDFNode);

	// Class definitions
	AddNodeComment(EventsDoc, RDFNode, "Class Definitions");
	FSLOwlDefinitions* ClassDefinitions = new FSLOwlDefinitions("owl:Class", "rdf:about");
	ClassDefinitions->AddObject("&knowrob;GraspingSomething");
	ClassDefinitions->AddObject("&knowrob_u;UnrealExperiment");
	ClassDefinitions->AddObject("&knowrob_u;TouchingSituation");
	ClassDefinitions->AddObject("&knowrob_u;KitchenEpisode");
	ClassDefinitions->AddObject("&knowrob_u;ParticleTranslation");
	ClassDefinitions->AddObject("&knowrob_u;FurnitureStateClosed");
	ClassDefinitions->AddObject("&knowrob_u;FurnitureStateHalfClosed");
	ClassDefinitions->AddObject("&knowrob_u;FurnitureStateOpened");
	ClassDefinitions->AddObject("&knowrob_u;FurnitureStateHalfOpened");
	ClassDefinitions->AddToDocument(EventsDoc, RDFNode);

	///////// EVENT INDIVIDUALS
	AddNodeComment(EventsDoc, RDFNode, "Event Individuals");
	// Add event individuals to RDF node
	for (const auto& FinishedEventItr : FinishedEvents)
	{
		FinishedEventItr->AddToDocument(EventsDoc, RDFNode);
	}

	///////// OBJECT INDIVIDUALS
	AddNodeComment(EventsDoc, RDFNode, "Object Individuals");
	for (const auto& ObjIndividualItr : ObjIndividualsMap)
	{
		ObjIndividualItr.Value->AddToDocument(EventsDoc, RDFNode);
	}

	/////////// TIMEPOINT INDIVIDUALS
	//AddNodeComment(EventsDoc, RDFNode, "Timepoint Individuals");
	//// Add time individuals to RDF node
	//for (const auto& TimepointIter : TimepointIndividuals)
	//{
	//	FSLUtils::AddNodeEntityWithProperty(EventsDoc, RDFNode,
	//		FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
	//			FSLUtils::FStringToChar("&log;" + TimepointIter)),
	//		FSLUtils::SLOwlTriple("rdf:type", "rdf:resource", "&knowrob;TimePoint"));
	//};

	///////// METADATA INDIVIDUAL
	AddNodeComment(EventsDoc, RDFNode, "Metadata Individual");
	// Add metadata to RDF node
	MetadataF->AddToDocument(EventsDoc, RDFNode);
	
	///////// ADD RDF NODE TO THE OWL DOC
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

// Add generic individual
void FSLEventsExporter::AddObjectIndividual(
	const FString ObjNamespace,
	const FString ObjName,
	const TArray<FSLOwlTriple>& Properties)
{
	if (!bListenToEvents)
	{
		return;
	}

	// Create unique name of the event
	const FString ObjUniqueName = ObjName + "_" + FSLUtils::GenerateRandomFString(4);
	// Init generic event
	FSLOwlObjectIndividual* ObjIndividual = new FSLOwlObjectIndividual(
		ObjNamespace, ObjUniqueName, Properties);
	// Add event to the finished events array
	ObjIndividualsMap.Add(ObjUniqueName, ObjIndividual);

	// Check for CRAMDesignators
	FSLEventsExporter::CheckForCRAMDesignators(Properties);
}

// Add generic event with array of properties
void FSLEventsExporter::AddFinishedEventIndividual(
	const FString& EventNs,
	const FString& EventName,
	const float StartTime,
	const float EndTime,
	const TArray<FSLOwlTriple>& Properties)
{
	if (!bListenToEvents)
	{
		return;
	}

	// Create unique name of the event
	const FString EventUniqueName = EventName + "_" + FSLUtils::GenerateRandomFString(4);
	// Init event
	FSLOwlEventIndividual* Event = new FSLOwlEventIndividual(
		EventNs, EventUniqueName, StartTime, EndTime, Properties);
	// Add event to the finished events array
	FinishedEvents.Add(Event);

	// Add start end time to the object individuals
	const FString StartTimeObj = Event->GetStartTimeProperty().GetRdfObject();
	const FString EndTimeObj = Event->GetEndTimeProperty().GetRdfObject();

	ObjIndividualsMap.Add(StartTimeObj, FSLEventsExporter::CreateTimeIndividual(StartTimeObj));
	ObjIndividualsMap.Add(EndTimeObj, FSLEventsExporter::CreateTimeIndividual(EndTimeObj));

	// Check for CRAMDesignators
	FSLEventsExporter::CheckForCRAMDesignators(Properties);
}

// Terminate all dangling events
void FSLEventsExporter::TerminateEvents(const float Timestamp)
{
	// Iterate all unfinished events
	for (const auto& NameToEvItr : NameToOpenedEventsMap)
	{
		// Add finishing time
		NameToEvItr.Value->SetEndTime(Timestamp);

		// Add endTime property
		FString EndTimeObject = "&log;" + FSLEventsExporter::GetAsKnowrobTs(Timestamp);
		FSLOwlTriple EndTimeProperty = FSLOwlTriple(
			"knowrob:endTime", "rdf:resource", EndTimeObject);
		MetadataF->AddProperty(EndTimeProperty);
		// Add time individual
		ObjIndividualsMap.Add(EndTimeObject,
			FSLEventsExporter::CreateTimeIndividual(EndTimeObject));

		// Add as subAction property to Metadata
		MetadataF->AddProperty(FSLOwlTriple("knowrob:subAction", "rdf:resource",
				NameToEvItr.Value->GetRdfObject()));

		// Add event to the finished events array
		FinishedEvents.Add(NameToEvItr.Value);

		UE_LOG(SemLogEvent, Log, TEXT("Terminate [%s]"), *NameToEvItr.Key);
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

	//// Add events to the timelines
	//for (const auto& FinishedEventItr : FinishedEvents)
	//{
	//	TimelineStr.Append("    [ '" + FinishedEventItr->UniqueName + "', "
	//		+ FString::SanitizeFloat(FinishedEventItr->Start) + ", "
	//		+ FString::SanitizeFloat(FinishedEventItr->End) + "],\n");
	//}


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

// Get the timepoint with namespace
FORCEINLINE const FString FSLEventsExporter::GetAsKnowrobTs(const float Timestamp)
{
	// KnowRob Timepoint
	return "timepoint_" + FString::SanitizeFloat(Timestamp);
}

// Create time object
FSLOwlObjectIndividual* FSLEventsExporter::CreateTimeIndividual(const FString TimeObject)
{
	return new FSLOwlObjectIndividual(TimeObject,
		TArray<FSLOwlTriple>{FSLOwlTriple("rdf:type", "rdf:resource", "&knowrob;TimePoint")});
};

// Check for designators in the properties
void FSLEventsExporter::CheckForCRAMDesignators(const TArray<FSLOwlTriple>& Properties)
{
	for (auto PropItr : Properties)
	{
		FString Sub = PropItr.GetRdfSubject();
		if(Sub.Contains("knowrob:goalLocation") ||
			Sub.Contains("knowrob:perceptionResult"))
		{
			FString Obj = PropItr.GetRdfObject();
			ObjIndividualsMap.Add(Obj, new FSLOwlObjectIndividual(Obj,
				TArray<FSLOwlTriple>{FSLOwlTriple("rdf:type", "rdf:resource", "&knowrob;CRAMDesignator")}));
		}
	}
}

// Init the metadata object individual of the episode
void FSLEventsExporter::InitMetadata(const float Timestamp)
{
	// Init metadata object individual
	MetadataF = new FSLOwlObjectIndividual("&log;", "UnrealExperiment_" + EpisodeUniqueTag);

	// Add class property
	MetadataF->AddProperty(FSLOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;UnrealExperiment"));

	// Add experiment unique name tag
	MetadataF->AddProperty(FSLOwlTriple(
		"knowrob:experiment", "rdf:datatype", "&xsd;string", EpisodeUniqueTag));

	// Add startTime property
	FString StartTimeObject = "&log;" + FSLEventsExporter::GetAsKnowrobTs(Timestamp);
	FSLOwlTriple StartTimeProperty = FSLOwlTriple(
		"knowrob:startTime", "rdf:resource", StartTimeObject);
	MetadataF->AddProperty(StartTimeProperty);
	// Add time individual
	ObjIndividualsMap.Add(StartTimeObject,
		FSLEventsExporter::CreateTimeIndividual(StartTimeObject));
}

// Finish metadata
void FSLEventsExporter::FinishMetadata(const float Timestamp)
{
	// Add startTime property
	FString EndTimeObject = "&log;" + FSLEventsExporter::GetAsKnowrobTs(Timestamp);
	FSLOwlTriple EndTimeProperty = FSLOwlTriple(
		"knowrob:endTime", "rdf:resource", EndTimeObject);
	MetadataF->AddProperty(EndTimeProperty);
	// Add time individual
	ObjIndividualsMap.Add(EndTimeObject,
		FSLEventsExporter::CreateTimeIndividual(EndTimeObject));
}

// Add document delcarations
void FSLEventsExporter::AddDocumentDeclarations(rapidxml::xml_document<>* Doc)
{
	///////// TYPE DECLARATION
	rapidxml::xml_node<> *DeclarationNode = Doc->allocate_node(rapidxml::node_declaration);
	// Create attibutes
	AddNodeAttribute(Doc, DeclarationNode, "version", "1.0");
	AddNodeAttribute(Doc, DeclarationNode, "encoding", "utf-8");
	// Add node to document
	Doc->append_node(DeclarationNode);

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
	rapidxml::xml_node<> *DoctypeNode = Doc->allocate_node(rapidxml::node_doctype, "", doctype);
	// Add node to document
	Doc->append_node(DoctypeNode);
}

// Add RDF Node attributes
void FSLEventsExporter::AddRDFNodeAttributes(rapidxml::xml_document<>* Doc, rapidxml::xml_node<>* RDFNode)
{
	// Add attributes
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:swrl", "http://www.w3.org/2003/11/swrl#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:owl", "http://www.w3.org/2002/07/owl#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:knowrob", "http://knowrob.org/kb/knowrob.owl#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	AddNodeAttribute(Doc, RDFNode,
		"xmlns:u-map", "http://knowrob.org/kb/u_map.owl#");
	AddNodeAttribute(Doc, RDFNode,
		"xml:base", "http://knowrob.org/kb/u_map.owl#");
}
