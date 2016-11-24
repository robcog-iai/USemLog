// Fill out your copyright notice in the Description page of Project Settings.

#include "USemLogPrivatePCH.h"
#include "SLUtils.h"
#include "SLMapExporter.h"

// Set default values
FSLMapExporter::FSLMapExporter()
{
	// Generate unique name
	MapUniqueName = "USemMap_" + FSLUtils::GenerateRandomFString(4);
}

// Write semantic map
void FSLMapExporter::WriteSemanticMap(
	const TMap<AActor*, FString>& ActToUniqueName,
	const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo,
	const FString Path)
{	
	UE_LOG(SemLogMap, Log, TEXT(" ** Writing semantic map [%s], with individuals:"), *Path);
	// Declare the xml semantic map document
	rapidxml::xml_document<>* SemMapDoc = new rapidxml::xml_document<>();

	// Add declaration of the xml map document
	FSLMapExporter::AddDoctype(SemMapDoc);

	// Semantic map RDF node
	rapidxml::xml_node<>* RDFNode = SemMapDoc->allocate_node(rapidxml::node_element, "rdf:RDF", "");
	
	// Add rdf attributes
	FSLMapExporter::AddRDFAttribures(SemMapDoc, RDFNode);

	// Import ontologies
	FSLMapExporter::ImportOntologies(SemMapDoc, RDFNode);

	// Add general definitions
	FSLMapExporter::AddGeneralDefinitions(SemMapDoc, RDFNode);

	// Add the semantic map individual
	FSLMapExporter::AddMapIndividual(SemMapDoc, RDFNode);

	// Add the semantic map events individuals
	FSLMapExporter::AddMapEventIndividuals(SemMapDoc, RDFNode, ActToUniqueName, ActToSemLogInfo);

	// Create string
	std::string RapidXmlString;
	rapidxml::print(std::back_inserter(RapidXmlString), *SemMapDoc, 0 /*rapidxml::print_no_indenting*/);
	FString OwlString = UTF8_TO_TCHAR(RapidXmlString.c_str());

	// Write string to file
	FFileHelper::SaveStringToFile(OwlString, *Path);
}

// Add the semantic map document declaration
inline void FSLMapExporter::AddDoctype(rapidxml::xml_document<>* SemMapDoc)
{
	///////// TYPE DECLARATION
	// Create declaration node <?xml version="1.0" encoding="utf-8"?>
	rapidxml::xml_node<> *DeclarationNode = SemMapDoc->allocate_node(rapidxml::node_declaration);
	// Create attibutes
	FSLUtils::AddNodeAttribute(SemMapDoc, DeclarationNode, "version", "1.0");
	FSLUtils::AddNodeAttribute(SemMapDoc, DeclarationNode, "encoding", "utf-8");
	// Add node to document
	SemMapDoc->append_node(DeclarationNode);

	///////// DOCTYPE
	// Doctype text
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
	rapidxml::xml_node<> *DoctypeNode = SemMapDoc->allocate_node(rapidxml::node_doctype, "", doctype);
	// Add node to document
	SemMapDoc->append_node(DoctypeNode);
}

// Add rdf node attributes
inline void FSLMapExporter::AddRDFAttribures(rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode)
{
	// Add attributes
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:swrl", "http://www.w3.org/2003/11/swrl#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:owl", "http://www.w3.org/2002/07/owl#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:knowrob", "http://knowrob.org/kb/knowrob.owl#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xmlns:u-map", "http://knowrob.org/kb/u_map.owl#");
	FSLUtils::AddNodeAttribute(SemMapDoc, RDFNode,
		"xml:base", "http://knowrob.org/kb/u_map.owl#");
}

// Import ontologies
inline void FSLMapExporter::ImportOntologies(rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode)
{
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Ontologies");
	// Create entity node with ontologies as properties
	TArray<FSLUtils::SLOwlTriple> Ontologies;
	Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_robcog/owl/knowrob_u.owl"));
	FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl"),
		Ontologies);
}

// Add general definitions
inline void FSLMapExporter::AddGeneralDefinitions(rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode)
{
	// Object property definitions
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Property Definitions");
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;describedInMap"));

	// Datatype property definitions
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Datatype Definitions");
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;depthOfObject"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;heightOfObject"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;widthOfObject"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorX"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorY"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorZ"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;pathToCadModel"));

	// Class definitions
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Class Definitions");
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob;SemanticEnvironmentMap"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob;SemanticMapPerception"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob;TimePoint"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Class", "rdf:about", "&knowrob;Vector"));
}

// Add semantic map individual
inline void FSLMapExporter::AddMapIndividual(
	rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode)
{
	// Semantic map individual
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Semantic Environment Map");
	// Add semantic map instance
	FSLUtils::AddNodeEntityWithProperty(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
			FSLUtils::FStringToChar("&u-map;" + MapUniqueName)),
		FSLUtils::SLOwlTriple("rdf:type", "rdf:resource", "&knowrob;SemanticEnvironmentMap"));

	// Timepoint individual
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Timepoint");
	// Add timepoint instance
	FSLUtils::AddNodeEntityWithProperty(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about", "&u-map;timepoint_0"),
		FSLUtils::SLOwlTriple("rdf:type", "rdf:resource", "&knowrob;Timepoint"));
}

// Add map event individuals
inline void FSLMapExporter::AddMapEventIndividuals(
	rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode, 
	const TMap<AActor*, FString>& ActToUniqueName,
	const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo)
{
	// Iterate 
	for (const auto& ActToSemLogItr : ActToSemLogInfo)
	{
		// Local copies of actor, name and unique name
		const AActor* CurrAct = ActToSemLogItr.Key;
		const FString ActName = CurrAct->GetName();
		const FString ActUniqueName = ActToUniqueName[CurrAct];
		// Get the class type from the semlog info
		FString ActClass = FSLUtils::GetPairArrayValue(ActToSemLogItr.Value, TEXT("Class"));

		UE_LOG(SemLogMap, Log, TEXT(" \t %s --> %s \t [Class = %s]"),
			*ActName,*ActUniqueName, *ActClass);

		// Transf unique name
		const FString TransfUniqueName = "Transformation_" + FSLUtils::GenerateRandomFString(4);

		// Loc and rotation as quat of the objects as strings, change from left hand to right hand coord
		const FVector Loc = CurrAct->GetActorLocation() / 100;
		const FString LocStr = FString::SanitizeFloat(Loc.X) + " "
			+ FString::SanitizeFloat(-Loc.Y) + " "
			+ FString::SanitizeFloat(Loc.Z);

		const FQuat Quat = CurrAct->GetActorQuat();
		const FString QuatStr = FString::SanitizeFloat(Quat.W) + " "
			+ FString::SanitizeFloat(-Quat.X) + " "
			+ FString::SanitizeFloat(Quat.Y) + " "
			+ FString::SanitizeFloat(-Quat.Z);

		// Object instance
		FSLUtils::AddNodeComment(SemMapDoc, RDFNode, FSLUtils::FStringToChar("Object " + ActUniqueName));

		// Array of object properties
		TArray<FSLUtils::SLOwlTriple> ObjProperties;
		// Add obj event properties
		ObjProperties.Add(FSLUtils::SLOwlTriple(
			"rdf:type", "rdf:resource", FSLUtils::FStringToChar("&knowrob;" + ActClass)));
		ObjProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:pathToCadModel", "rdf:datatype", "&xsd; string",
			FSLUtils::FStringToChar("package://sim/unreal/" + ActClass + ".dae")));
		ObjProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:describedInMap", "rdf:resource",
			FSLUtils::FStringToChar(FSLUtils::FStringToChar("&u-map;" + MapUniqueName))));
		// Add instance with properties
		FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
			FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
				FSLUtils::FStringToChar("&log;" + ActUniqueName)), ObjProperties);

		// Map perception unique name
		const FString MapPerceptionUniqueName = "SemanticMapPerception_" + FSLUtils::GenerateRandomFString(4);

		// Map perception properties
		TArray<FSLUtils::SLOwlTriple> MapPerceptionProperties;
		// Add obj event properties
		MapPerceptionProperties.Add(FSLUtils::SLOwlTriple(
			"rdf:type", "rdf:resource", "&knowrob;SemanticMapPerception"));
		MapPerceptionProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:eventOccursAt", "rdf:resource",
			FSLUtils::FStringToChar("&u-map;" + TransfUniqueName)));
		MapPerceptionProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:startTime", "rdf:resource", "&u-map;timepoint_0"));
		MapPerceptionProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:objectActedOn", "rdf:resource",
			FSLUtils::FStringToChar("&log;" + ActUniqueName)));
		// Add instance with properties
		FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
			FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
				FSLUtils::FStringToChar(MapPerceptionUniqueName)),
			MapPerceptionProperties);

		// Transformation properties
		TArray<FSLUtils::SLOwlTriple> TransfProperties;
		// Add obj event properties
		TransfProperties.Add(FSLUtils::SLOwlTriple(
			"rdf:type", "rdf:resource", "&knowrob;Transformation"));
		TransfProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:quaternion", "rdf:datatype", "&xsd;string", FSLUtils::FStringToChar(QuatStr)));
		TransfProperties.Add(FSLUtils::SLOwlTriple(
			"knowrob:translation", "rdf:datatype", "&xsd;string", FSLUtils::FStringToChar(LocStr)));
		// Add instance with properties
		FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
			FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
				FSLUtils::FStringToChar("&u-map;" + TransfUniqueName)),
			TransfProperties);
	}

	///////// ADD RDF TO OWL DOC
	SemMapDoc->append_node(RDFNode);
}