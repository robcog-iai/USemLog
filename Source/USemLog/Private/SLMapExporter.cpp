// Fill out your copyright notice in the Description page of Project Settings.

#include "USemLogPrivatePCH.h"
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
	const TMap<FString, UInstancedStaticMeshComponent*>& FoliageClassNameToComponent,
	const TMap<UInstancedStaticMeshComponent*, TArray<TPair<FBodyInstance*, FString>>>& FoliageComponentToUniqueNameArray,
	const TMap<FString, USceneComponent*>& RoadCompNameToComponent,
	const TMap<FString, FString>& RoadComponentNameToUniqueName,
	const FString& RoadUniqueName,
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
	FSLMapExporter::AddOntologies(SemMapDoc, RDFNode);

	// Add general definitions
	FSLMapExporter::AddGeneralDefinitions(SemMapDoc, RDFNode);

	// Add the semantic map individual
	FSLMapExporter::AddMapIndividual(SemMapDoc, RDFNode, RoadUniqueName, RoadCompNameToComponent.Num());

	// Add the semantic map events individuals
	const FString CSVFile = FSLMapExporter::AddAllMapEventIndividuals(SemMapDoc, RDFNode,
		ActToUniqueName,
		ActToSemLogInfo,
		FoliageClassNameToComponent,
		FoliageComponentToUniqueNameArray,
		RoadCompNameToComponent,
		RoadComponentNameToUniqueName,
		RoadUniqueName);

	// Write string to file
	FFileHelper::SaveStringToFile(CSVFile, *(Path + ".csv"));

	// Create string
	std::string RapidXmlString;
	rapidxml::print(std::back_inserter(RapidXmlString), *SemMapDoc, 0 /*rapidxml::print_no_indenting*/);
	FString OwlString = UTF8_TO_TCHAR(RapidXmlString.c_str());

	// Write string to file
	FFileHelper::SaveStringToFile(OwlString, *Path);
}

// Add the semantic map document declaration
FORCEINLINE void FSLMapExporter::AddDoctype(rapidxml::xml_document<>* SemMapDoc)
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
FORCEINLINE void FSLMapExporter::AddRDFAttribures(rapidxml::xml_document<>* SemMapDoc,
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
FORCEINLINE void FSLMapExporter::AddOntologies(rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode)
{
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, "Ontologies");
	// Create entity node with ontologies as properties
	TArray<FSLUtils::SLOwlTriple> Ontologies;
	Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	//Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://knowrob_robcog/owl/knowrob_u.owl"));
	Ontologies.Add(FSLUtils::SLOwlTriple("owl:imports", "rdf:resource", "package://sherpa_world/owl/knowrob_sherpa.owl"));
	FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl"),
		Ontologies);
}

// Add general definitions
FORCEINLINE void FSLMapExporter::AddGeneralDefinitions(rapidxml::xml_document<>* SemMapDoc,
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
FORCEINLINE void FSLMapExporter::AddMapIndividual(
	rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode,
	const FString& RoadUniqueName,
	int32 NrOfRoadSegments)
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

	// Semantic map individual
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, FSLUtils::FStringToChar("Mountain road " + RoadUniqueName));

	// Array of object properties
	TArray<FSLUtils::SLOwlTriple> ObjProperties;
	// Add obj event properties
	ObjProperties.Add(FSLUtils::SLOwlTriple("rdf:type", "rdf:resource", "&knowrob;Road"));
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:nrOfSegments", "rdf:datatype", "&xsd;int",
		FSLUtils::FStringToChar(FString::FromInt(NrOfRoadSegments))));
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:describedInMap", "rdf:resource",
		FSLUtils::FStringToChar("&u-map;" + MapUniqueName)));

	// Add road instance
	FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
			FSLUtils::FStringToChar("&log;" + RoadUniqueName)), ObjProperties);
}

// Add map event individuals
FORCEINLINE const FString FSLMapExporter::AddAllMapEventIndividuals(
	rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode, 
	const TMap<AActor*, FString>& ActToUniqueName,
	const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo,
	const TMap<FString, UInstancedStaticMeshComponent*>& FoliageClassNameToComponent,
	const TMap<UInstancedStaticMeshComponent*, TArray<TPair<FBodyInstance*, FString>>>& FoliageComponentToUniqueNameArray,
	const TMap<FString, USceneComponent*>& RoadCompNameToComponent,
	const TMap<FString, FString>& RoadComponentNameToUniqueName,
	const FString& RoadUniqueName)
{
	FString CSVString;
	UE_LOG(SemLogMap, Log, TEXT(" ** Adding the semantic map individuals: "));

	UE_LOG(SemLogMap, Log, TEXT(" \t Actors: "));
	// Iterate 
	for (const auto& ActToSemLogItr : ActToSemLogInfo)
	{		
		// Local copies of actor, name and unique name
		const AActor* CurrAct = ActToSemLogItr.Key;
		const FString ActName = CurrAct->GetName();
		const FString ActUniqueName = ActToUniqueName[CurrAct];
		// Get the class type from the semlog info
		const FString ActClass = FSLUtils::GetPairArrayValue(ActToSemLogItr.Value, TEXT("Class"));

		UE_LOG(SemLogMap, Log, TEXT(" \t\t %s --> %s \t\t [Class = %s]"),
			*ActName,*ActUniqueName, *ActClass);
		// Loc and rotation as quat of the objects as strings, change from left hand to right hand coord
		const FVector Loc = CurrAct->GetActorLocation() / 100;
		const FQuat Quat = CurrAct->GetActorQuat();
		// Get the bounds
		FBox BoundingBox = CurrAct->GetComponentsBoundingBox();

		// Add the individual
		FSLMapExporter::AddMapIndividual(SemMapDoc, RDFNode, Loc, Quat, BoundingBox, ActClass, ActUniqueName);
	}
	
	UE_LOG(SemLogMap, Log, TEXT(" \t Foliage: "));
	// Iterate foliage if available
	for (const auto& FoliageClassNameToCompItr : FoliageClassNameToComponent)
	{
		// Get the component class name
		const FString CompClassName = FoliageClassNameToCompItr.Key;

		// Get the current component unique names array
		TArray<TPair<FBodyInstance*, FString>> CurrCompUniqueNameArray =
			*FoliageComponentToUniqueNameArray.Find(FoliageClassNameToCompItr.Value);
		UE_LOG(SemLogMap, Log, TEXT(" \t\t Class = %s, number of objects: %i"),
			*CompClassName, CurrCompUniqueNameArray.Num());
		// Iterate the bodies of the current foliage component
		for (const auto& UniqueNameItr : CurrCompUniqueNameArray)
		{
			// The unique name of the body
			const FString BodyUniqueName = UniqueNameItr.Value;
			//UE_LOG(SemLogMap, Log, TEXT(" \t\t %s \t [Class = %s]"),
			//	*BodyUniqueName, *CompClassName);
	
			// Loc and rotation as quat of the objects as strings, change from left hand to right hand coord
			FTransform BodyTransf = UniqueNameItr.Key->GetUnrealWorldTransform();
			const FVector Loc = BodyTransf.GetLocation() / 100;
			const FQuat Quat = BodyTransf.GetRotation();
			// Get the bounds
			const FBox BoundingBox = UniqueNameItr.Key->GetBodyBounds();

			// Add the individual
			FSLMapExporter::AddMapIndividual(SemMapDoc, RDFNode, Loc, Quat, BoundingBox, CompClassName, BodyUniqueName);

			CSVString.Append(BodyUniqueName + "," 
				+ FString::SanitizeFloat(Loc.X) + "," 
				+ FString::SanitizeFloat(-Loc.Y) + ","
				+ FString::SanitizeFloat(Loc.Z) + "\n");
		}
	}

	UE_LOG(SemLogMap, Log, TEXT(" \t Road: "));
	// Iterate road if available
	for (const auto& RoadCompNameToCompItr : RoadCompNameToComponent)
	{
		// Local copies of actor, name and unique name
		const USceneComponent* CurrComp = RoadCompNameToCompItr.Value;
		const FString RoadSegmentName = RoadCompNameToCompItr.Key;
		const FString RoadSegmentUniqueName = RoadComponentNameToUniqueName[RoadSegmentName];
		// Get the class type from the semlog info
		const FString RoadClass = "RoadSegment";

		UE_LOG(SemLogMap, Log, TEXT(" \t\t %s --> %s \t\t [Class = %s]"),
			*RoadSegmentName, *RoadSegmentUniqueName, *RoadClass);
		// Loc and rotation as quat of the objects as strings, change from left hand to right hand coord
		const FVector Loc = CurrComp->GetComponentLocation() / 100;
		const FQuat Quat = CurrComp->GetComponentQuat();
		// Get the bounds
		FBox BoundingBox = FBox(0.f);

		// Add extra properties
		TArray<FSLUtils::SLOwlTriple> ExtraProperties;
		ExtraProperties.Add(FSLUtils::SLOwlTriple("knowrob:partOf", "rdf:resource",
			FSLUtils::FStringToChar("&log;" + RoadUniqueName)));

		// Add the individual
		FSLMapExporter::AddMapIndividual(SemMapDoc, RDFNode, Loc, Quat, BoundingBox, RoadClass, RoadSegmentUniqueName,
			ExtraProperties);
	}

	///////// ADD RDF TO OWL DOC
	SemMapDoc->append_node(RDFNode);

	return CSVString;
}

// Add a map individual
FORCEINLINE void FSLMapExporter::AddMapIndividual(
	rapidxml::xml_document<>* SemMapDoc,
	rapidxml::xml_node<>* RDFNode,
	const FVector Loc,
	const FQuat Quat,
	const FBox Box,
	const FString& ClassName,
	const FString& UniqueName,
	const TArray<FSLUtils::SLOwlTriple>& ExtraProperties)
{
	const FString LocStr = FString::SanitizeFloat(Loc.X) + " "
		+ FString::SanitizeFloat(-Loc.Y) + " "
		+ FString::SanitizeFloat(Loc.Z);
	const FString QuatStr = FString::SanitizeFloat(Quat.W) + " "
		+ FString::SanitizeFloat(-Quat.X) + " "
		+ FString::SanitizeFloat(Quat.Y) + " "
		+ FString::SanitizeFloat(-Quat.Z);
	const FVector BoundingBoxSize = Box.GetSize() / 100;

	// Transf unique name
	const FString TransfUniqueName = "Transformation_" + FSLUtils::GenerateRandomFString(4);

	// Foliage object instance
	FSLUtils::AddNodeComment(SemMapDoc, RDFNode, FSLUtils::FStringToChar("Foliage object " + UniqueName));

	// Array of object properties
	TArray<FSLUtils::SLOwlTriple> ObjProperties;
	// Add obj event properties
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"rdf:type", "rdf:resource", 
		FSLUtils::FStringToChar("&knowrob;" + ClassName)));
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:depthOfObject", "rdf:datatype", "&xsd;double",
		FSLUtils::FStringToChar(FString::SanitizeFloat(BoundingBoxSize.X))));
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:widthOfObject", "rdf:datatype", "&xsd;double",
		FSLUtils::FStringToChar(FString::SanitizeFloat(BoundingBoxSize.Y))));
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:heightOfObject", "rdf:datatype", "&xsd;double",
		FSLUtils::FStringToChar(FString::SanitizeFloat(BoundingBoxSize.Z))));
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:pathToCadModel", "rdf:datatype", "&xsd;string",
		FSLUtils::FStringToChar("package://sim/unreal/" + ClassName + ".dae")));	
	// Add extra properties
	for(const auto& ExtraProperty : ExtraProperties)
	{
		ObjProperties.Add(ExtraProperty);
	}
	ObjProperties.Add(FSLUtils::SLOwlTriple(
		"knowrob:describedInMap", "rdf:resource",
		FSLUtils::FStringToChar("&u-map;" + MapUniqueName)));
	// Add instance with properties
	FSLUtils::AddNodeEntityWithProperties(SemMapDoc, RDFNode,
		FSLUtils::SLOwlTriple("owl:NamedIndividual", "rdf:about",
			FSLUtils::FStringToChar("&log;" + UniqueName)), ObjProperties);

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
		FSLUtils::FStringToChar("&log;" + UniqueName)));
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