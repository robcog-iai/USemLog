// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"
#include "SLUtils.h"
#include "RSemMapExporter.h"

// Set default values
FRSemMapExporter::FRSemMapExporter()
{
	// Generate unique name
	UniqueName = "USemMap_" + FRUtils::GenerateRandomFString(4);
}

// Destructor
FRSemMapExporter::~FRSemMapExporter()
{
}

// Write semantic map
void FRSemMapExporter::WriteSemanticMap(
	const TMap<AStaticMeshActor*, FString>& DynamicActPtrToUniqNameMap,
	const TMap<AStaticMeshActor*, FString>& StaticActPtrToUniqNameMap,
	const TMap<AActor*, FString>& ActorToClassTypeMap,
	const TPair<USceneComponent*, FString> CameraToUniqueName,
	const FString Path)
{
	///////// DOC
	// Semantic map document
	rapidxml::xml_document<>* SemMapDoc = new rapidxml::xml_document<>();

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

	///////// RDF NODE
	// Semantic map RDF node
	rapidxml::xml_node<>* RDFNode = SemMapDoc->allocate_node(rapidxml::node_element, "rdf:RDF", "");

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

	///////// ONTOLOGY IMPORT
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Ontologies");
	// Create entity node with ontologies as properties
	TArray<FSLUtils::ROwlTriple> Ontologies;
	Ontologies.Add(FSLUtils::ROwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	Ontologies.Add(FSLUtils::ROwlTriple("owl:imports", "rdf:resource", "package://knowrob_robcog/owl/knowrob_u.owl"));
	FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl"),
		Ontologies);

	///////// GENERAL DEFINITIONS
	// Object property definitions
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Property Definitions");
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:ObjectProperty", "rdf:about", "&knowrob;describedInMap"));

	// Datatype property definitions
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Datatype Definitions");
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;depthOfObject"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;heightOfObject"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;widthOfObject"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorX"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorY"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorZ"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:DatatypeProperty", "rdf:about", "&knowrob;pathToCadModel"));

	// Class definitions
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Class Definitions");
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:Class", "rdf:about", "&knowrob;SemanticEnvironmentMap"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:Class", "rdf:about", "&knowrob;SemanticMapPerception"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:Class", "rdf:about", "&knowrob;TimePoint"));
	FSLUtils::AddNodeTriple(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:Class", "rdf:about", "&knowrob;Vector"));


	///////// EVENT INDIVIDUALS
	// Semantic map individual
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Semantic Environment Map");
	// Add semantic map instance
	FSLUtils::AddNodeEntityWithProperty(SemMapDoc, RDFNode, 
		FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about", 
			FRUtils::FStringToChar("&u-map;" + UniqueName)), 
		FSLUtils::ROwlTriple("rdf:type", "rdf:resource", "&knowrob;SemanticEnvironmentMap"));

	// Timepoint individual
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Timepoint");
	// Add timepoint instance
	FSLUtils::AddNodeEntityWithProperty(SemMapDoc, RDFNode,
		FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about", "&u-map;timepoint_0"),
		FSLUtils::ROwlTriple("rdf:type", "rdf:resource", "&knowrob;Timepoint"));
	
	UE_LOG(SemLogMap, Warning, TEXT(" *** "));
	UE_LOG(SemLogMap, Warning, TEXT("Semantic map components:"));

	// Lambda to add actors to the semantic map as perception events
	auto AddActorsToSemMapLambda = [SemMapDoc, RDFNode, this](
		const TMap<AStaticMeshActor*, FString>& ActPtrToUniqNameMap,
		const TMap<AActor*, FString>& ActorToClassTypeMap)
	{
		for (const auto ActPtrToUniqNameItr : ActPtrToUniqNameMap)
		{
			// Local copies of name and unique name
			const FString ActName = ActPtrToUniqNameItr.Key->GetName();
			const FString ActUniqueName = ActPtrToUniqNameItr.Value;
			const FString ActClass = ActorToClassTypeMap[ActPtrToUniqNameItr.Key];
			UE_LOG(SemLogMap, Warning, TEXT("\t%s [%s] -> %s "), *ActName, *ActClass, *ActUniqueName);

			// Transf unique name
			const FString TransfUniqueName = "Transformation_" + FRUtils::GenerateRandomFString(4);

			// Loc and rotation as quat of the objects as strings, change from left hand to right hand coord
			const FVector Loc = ActPtrToUniqNameItr.Key->GetActorLocation() / 100;
			const FString LocStr = FString::SanitizeFloat(Loc.X) + " "
				+ FString::SanitizeFloat(-Loc.Y) + " "
				+ FString::SanitizeFloat(Loc.Z);

			const FQuat Quat = ActPtrToUniqNameItr.Key->GetActorQuat();
			const FString QuatStr = FString::SanitizeFloat(Quat.W) + " "
				+ FString::SanitizeFloat(-Quat.X) + " "
				+ FString::SanitizeFloat(Quat.Y) + " "
				+ FString::SanitizeFloat(-Quat.Z);

			// Object instance
			FSLUtils::AddNodeComment(SemMapDoc, RDFNode, FRUtils::FStringToChar("Object " + ActUniqueName));

			// Array of object properties
			TArray<FSLUtils::ROwlTriple> ObjProperties;
			// Add obj event properties
			ObjProperties.Add(FSLUtils::ROwlTriple(
				"rdf:type", "rdf:resource", FRUtils::FStringToChar("&knowrob;" + ActClass)));
			ObjProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:pathToCadModel", "rdf:datatype", "&xsd; string",
				FRUtils::FStringToChar("package://sim/unreal/" + ActClass + ".dae")));
			ObjProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:describedInMap", "rdf:resource",
				FRUtils::FStringToChar(FRUtils::FStringToChar("&u-map;" + UniqueName))));
			// Add instance with properties
			FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
				FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about",
					FRUtils::FStringToChar("&log;" + ActUniqueName)), ObjProperties);

			// Map perception unique name
			const FString MapPerceptionUniqueName = "SemanticMapPerception_" + FRUtils::GenerateRandomFString(4);

			// Map perception properties
			TArray<FSLUtils::ROwlTriple> MapPerceptionProperties;
			// Add obj event properties
			MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
				"rdf:type", "rdf:resource", "&knowrob;SemanticMapPerception"));
			MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:eventOccursAt", "rdf:resource",
				FRUtils::FStringToChar("&u-map;" + TransfUniqueName)));
			MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:startTime", "rdf:resource", "&u-map;timepoint_0"));
			MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:objectActedOn", "rdf:resource",
				FRUtils::FStringToChar("&log;" + ActUniqueName)));
			// Add instance with properties
			FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
				FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about",
					FRUtils::FStringToChar(MapPerceptionUniqueName)),
				MapPerceptionProperties);

			// Transformation properties
			TArray<FSLUtils::ROwlTriple> TransfProperties;
			// Add obj event properties
			TransfProperties.Add(FSLUtils::ROwlTriple(
				"rdf:type", "rdf:resource", "&knowrob;Transformation"));
			TransfProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:quaternion", "rdf:datatype", "&xsd;string", FRUtils::FStringToChar(QuatStr)));
			TransfProperties.Add(FSLUtils::ROwlTriple(
				"knowrob:translation", "rdf:datatype", "&xsd;string", FRUtils::FStringToChar(LocStr)));
			// Add instance with properties
			FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
				FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about",
					FRUtils::FStringToChar("&u-map;" + TransfUniqueName)),
				TransfProperties);
		}
	};

	// Add dynamic and static actors initial position to the map
	AddActorsToSemMapLambda(DynamicActPtrToUniqNameMap, ActorToClassTypeMap);
	AddActorsToSemMapLambda(StaticActPtrToUniqNameMap, ActorToClassTypeMap);

	// Add camera to the map
	if (CameraToUniqueName.Key)
	{
		const FString CameraName = CameraToUniqueName.Key->GetName();
		const FString CameraUniqueName = CameraToUniqueName.Value;
		const FString CameraClass = CameraToUniqueName.Key->GetName();
		UE_LOG(SemLogMap, Warning, TEXT("\t%s [%s] -> %s "), *CameraName, *CameraUniqueName, *CameraClass);

		// Transf unique name
		const FString TransfUniqueName = "Transformation_" + FRUtils::GenerateRandomFString(4);

		// Loc and rotation as quat of the objects as strings, change from left hand to right hand coord
		const FVector Loc = CameraToUniqueName.Key->GetComponentLocation() / 100;
		const FString LocStr = FString::SanitizeFloat(Loc.X) + " "
			+ FString::SanitizeFloat(-Loc.Y) + " "
			+ FString::SanitizeFloat(Loc.Z);

		const FQuat Quat = CameraToUniqueName.Key->GetComponentQuat();
		const FString QuatStr = FString::SanitizeFloat(Quat.W) + " "
			+ FString::SanitizeFloat(-Quat.X) + " "
			+ FString::SanitizeFloat(Quat.Y) + " "
			+ FString::SanitizeFloat(-Quat.Z);

		// Camera Object instance
		FSLUtils::AddNodeComment(SemMapDoc, RDFNode, FRUtils::FStringToChar("Object " + CameraUniqueName));

		// Array of object properties
		TArray<FSLUtils::ROwlTriple> ObjProperties;
		// Add obj event properties
		ObjProperties.Add(FSLUtils::ROwlTriple(
			"rdf:type", "rdf:resource", FRUtils::FStringToChar("&knowrob;" + CameraClass)));
		ObjProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:pathToCadModel", "rdf:datatype", "&xsd; string",
			FRUtils::FStringToChar("package://sim/unreal/" + CameraClass + ".dae")));
		ObjProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:describedInMap", "rdf:resource",
			FRUtils::FStringToChar(FRUtils::FStringToChar("&u-map;" + UniqueName))));
		// Add instance with properties
		FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
			FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about",
				FRUtils::FStringToChar("&log;" + CameraUniqueName)), ObjProperties);

		// Map perception unique name
		const FString MapPerceptionUniqueName = "SemanticMapPerception_" + FRUtils::GenerateRandomFString(4);

		// Map perception properties
		TArray<FSLUtils::ROwlTriple> MapPerceptionProperties;
		// Add obj event properties
		MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
			"rdf:type", "rdf:resource", "&knowrob;SemanticMapPerception"));
		MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:eventOccursAt", "rdf:resource",
			FRUtils::FStringToChar("&u-map;" + TransfUniqueName)));
		MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:startTime", "rdf:resource", "&u-map;timepoint_0"));
		MapPerceptionProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:objectActedOn", "rdf:resource",
			FRUtils::FStringToChar("&log;" + CameraUniqueName)));
		// Add instance with properties
		FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
			FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about",
				FRUtils::FStringToChar(MapPerceptionUniqueName)),
			MapPerceptionProperties);

		// Camera Transformation properties
		TArray<FSLUtils::ROwlTriple> TransfProperties;
		// Add obj event properties
		TransfProperties.Add(FSLUtils::ROwlTriple(
			"rdf:type", "rdf:resource", "&knowrob;Transformation"));
		TransfProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:quaternion", "rdf:datatype", "&xsd;string", FRUtils::FStringToChar(QuatStr)));
		TransfProperties.Add(FSLUtils::ROwlTriple(
			"knowrob:translation", "rdf:datatype", "&xsd;string", FRUtils::FStringToChar(LocStr)));
		// Add instance with properties
		FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
			FSLUtils::ROwlTriple("owl:NamedIndividual", "rdf:about",
				FRUtils::FStringToChar("&u-map;" + TransfUniqueName)),
			TransfProperties);
	}

	///////// ADD RDF TO OWL DOC
	SemMapDoc->append_node(RDFNode);

	// Create string
	std::string RapidXmlString;
	rapidxml::print(std::back_inserter(RapidXmlString), *SemMapDoc, 0 /*rapidxml::print_no_indenting*/);
	FString OwlString = UTF8_TO_TCHAR(RapidXmlString.c_str());

	// Write string to file
	FFileHelper::SaveStringToFile(OwlString, *Path);
}

// Get unique name
FString FRSemMapExporter::GetUniqueName()
{
	return UniqueName;
}
