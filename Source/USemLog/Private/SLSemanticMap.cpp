// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSemanticMap.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

using namespace SLOwl;

// Constructor
FSLSemanticMap::FSLSemanticMap()
{
	LogDirectory = TEXT("SemLog");
}

// Constructor with init
FSLSemanticMap::FSLSemanticMap(UWorld* World, const FString InDirectory)
{
	LogDirectory = InDirectory;
	Generate(World);
}

// Destructor
FSLSemanticMap::~FSLSemanticMap()
{
}

// Generate semantic map from world
void FSLSemanticMap::Generate(UWorld* World)
{
	// Constants
	const FPrefixName RdfPrefix("rdf", "RDF");
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName OwlImports("owl", "imports");
	const FPrefixName OwlObjectProperty("owl", "ObjectProperty");
	const FPrefixName OwlDatatypeProperty("owl", "DatatypeProperty");
	const FPrefixName OwlClass("owl", "Class");

	// XML declaration
	SemMap.SetDeclaration(TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>"));

	// Document type definition
	TMap<FString, FString> EntityPairs;
	EntityPairs.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	EntityPairs.Add("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	EntityPairs.Add("owl", "http://www.w3.org/2002/07/owl#");
	EntityPairs.Add("xsd", "http://www.w3.org/2001/XMLSchema#");
	EntityPairs.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	// TODO contd.
	SemMap.SetEntityDTD(FEntityDTD(RdfPrefix, EntityPairs));

	// Create root
	TArray<FAttribute> RootAttributes;
	RootAttributes.Add(FAttribute(FPrefixName("xmlns", "computable"),
		FAttributeValue("http://knowrob.org/kb/computable.owl#")));
	RootAttributes.Add(FAttribute(FPrefixName("xmlns", "swrl"),
		FAttributeValue("http://www.w3.org/2003/11/swrl#")));
	RootAttributes.Add(FAttribute(FPrefixName("xmlns", "rdf"),
		FAttributeValue("http://www.w3.org/1999/02/22-rdf-syntax-ns#")));
	RootAttributes.Add(FAttribute(FPrefixName("xmlns", "owl"),
		FAttributeValue("http://www.w3.org/2002/07/owl#")));
	// TODO contd.
	
	SemMap.SetRoot(FNode(RdfPrefix, RootAttributes));

	// Create ontology import node
	FNode ImportsNode(FPrefixName("owl", "Ontology"),
		FAttribute(RdfAbout, FAttributeValue("http://knowrob.org/kb/u_map.owl")));
	ImportsNode.SetComment("Ontologies");
	TArray<FNode> OntologyNodes;
	OntologyNodes.Add(FNode(OwlImports, 
		FAttribute(RdfResource, FAttributeValue("package://knowrob_common/owl/knowrob.owl"))));
	OntologyNodes.Add(FNode(OwlImports,
		FAttribute(RdfResource, FAttributeValue("package://knowrob_common/owl/knowrob_u.owl"))));
	ImportsNode.AddChildNodes(OntologyNodes);
	SemMap.GetRoot().AddChildNode(ImportsNode);

	// Create property definitions
	TArray<FNode> PropertyNodes;
	FNode FirstPropertyWithComment = FNode(OwlObjectProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "describedInMap")));
	FirstPropertyWithComment.SetComment("Property Definitions");
	PropertyNodes.Add(FirstPropertyWithComment);
	PropertyNodes.Add(FNode(OwlObjectProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "describedInMap2"))));
	SemMap.GetRoot().AddChildNodes(PropertyNodes);

	// Create datatype definitions
	TArray<FNode> DatatypeNodes;
	FNode FirstDatatypeWithComment = FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "depthOfObject")));
	FirstDatatypeWithComment.SetComment("Datatype Definitions");
	DatatypeNodes.Add(FirstDatatypeWithComment);
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "heightOfObject"))));
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "heightOfObject"))));
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "widthOfObject"))));
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "vectorX"))));
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "vectorY"))));
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "vectorZ"))));
	DatatypeNodes.Add(FNode(OwlDatatypeProperty,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "pathToCadModel"))));
	SemMap.GetRoot().AddChildNodes(DatatypeNodes);

	// Class definitions
	TArray<FNode> ClassNodes;
	FNode FirstClassWithComment = FNode(OwlClass,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "SemanticEnvironmentMap")));
	FirstClassWithComment.SetComment("Class Definitions");
	ClassNodes.Add(FirstClassWithComment);
	ClassNodes.Add(FNode(OwlClass,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "SLMapPerception"))));
	ClassNodes.Add(FNode(OwlClass,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "TimePoint"))));
	ClassNodes.Add(FNode(OwlClass,
		FAttribute(RdfAbout, FAttributeValue("knowrob", "Vector"))));

	SemMap.GetRoot().AddChildNodes(ClassNodes);
}

// Write semantic map to file
bool FSLSemanticMap::WriteToFile(const FString& Filename)
{
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/") + Filename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);

	return FFileHelper::SaveStringToFile(SemMap.ToString(), *FullFilePath);
}