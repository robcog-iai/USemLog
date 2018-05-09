// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SemanticMap/SLSemanticMap.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Tags.h"
#include "Ids.h"
#include "Conversions.h"

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
	// Prefix name constants
	const FPrefixName RdfPrefix("rdf", "RDF");
	
	// XML declaration
	SemMap.SetDeclaration(TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>"));

	// Get default document type definitions 
	FEntityDTD DefaultDTD;
	GetDefaultDTD(DefaultDTD);
	SemMap.SetEntityDTD(DefaultDTD);

	// Get default owl namespace declarations
	TArray<FAttribute> DefaultNamespaces;
	GetDefaultNamespaces(DefaultNamespaces);	
	SemMap.SetRoot(FNode(RdfPrefix, DefaultNamespaces));

	// Create ontology import node
	FNode ImportsNode;
	GetDefaultOntologyImports(ImportsNode);
	SemMap.GetRoot().AddChildNode(ImportsNode);

	// Create property definitions
	TArray<FNode> PropertyNodes;
	GetDefaultPropertyDefinitions(PropertyNodes);
	SemMap.GetRoot().AddChildNodes(PropertyNodes);

	// Create datatype definitions
	TArray<FNode> DatatypeNodes;
	GetDefaultDatatypeDefinitions(DatatypeNodes);
	SemMap.GetRoot().AddChildNodes(DatatypeNodes);

	// Class definitions
	TArray<FNode> ClassNodes;
	GetDefaultClassDefinitions(ClassNodes);
	SemMap.GetRoot().AddChildNodes(ClassNodes);

	// Add entries
	AddEntries(World);
}

// Get default document type definitions
void FSLSemanticMap::GetDefaultDTD(FEntityDTD& OutDTD)
{
	// DOCTYPE name
	OutDTD.Root = FPrefixName("rdf", "RDF");

	// Declarations
	OutDTD.EntityPairs.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	OutDTD.EntityPairs.Add("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	OutDTD.EntityPairs.Add("owl", "http://www.w3.org/2002/07/owl#");
	OutDTD.EntityPairs.Add("xsd", "http://www.w3.org/2001/XMLSchema#");
	OutDTD.EntityPairs.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	// TODO contd.	
}

// Get default namespaces
void FSLSemanticMap::GetDefaultNamespaces(TArray<FAttribute>& OutNamespaces)
{
	OutNamespaces.Add(FAttribute(FPrefixName("xmlns", "computable"),
		FAttributeValue("http://knowrob.org/kb/computable.owl#")));
	OutNamespaces.Add(FAttribute(FPrefixName("xmlns", "swrl"),
		FAttributeValue("http://www.w3.org/2003/11/swrl#")));
	OutNamespaces.Add(FAttribute(FPrefixName("xmlns", "rdf"),
		FAttributeValue("http://www.w3.org/1999/02/22-rdf-syntax-ns#")));
	OutNamespaces.Add(FAttribute(FPrefixName("xmlns", "owl"),
		FAttributeValue("http://www.w3.org/2002/07/owl#")));
	// TODO contd.
}

// Get default ontology imports
void FSLSemanticMap::GetDefaultOntologyImports(FNode& OutImports)
{
	// Prefix name constants
	const FPrefixName OwlImports("owl", "imports");
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfResource("rdf", "resource");

	// Create ontology import node
	OutImports.SetName(FPrefixName("owl", "Ontology"));
	OutImports.AddAttribute(FAttribute(RdfAbout, FAttributeValue("http://knowrob.org/kb/u_map.owl")));
	OutImports.SetComment("Ontologies");

	// Add child import nodes
	OutImports.AddChildNode(FNode(OwlImports,
		FAttribute(RdfResource, FAttributeValue("package://knowrob_common/owl/knowrob.owl"))));
	OutImports.AddChildNode(FNode(OwlImports,
		FAttribute(RdfResource, FAttributeValue("package://knowrob_common/owl/knowrob_u.owl"))));
}

// Get default property definitions
void FSLSemanticMap::GetDefaultPropertyDefinitions(TArray<FNode>& OutNodes)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName OwlOP("owl", "ObjectProperty");

	// Add comment to first node
	FNode NodeWithComment = FNode(OwlOP, FAttribute(RdfAbout, FAttributeValue("knowrob", "describedInMap")));
	NodeWithComment.SetComment("Property Definitions");
	OutNodes.Add(NodeWithComment);

	OutNodes.Add(FNode(OwlOP, FAttribute(RdfAbout, FAttributeValue("knowrob", "describedInMap2"))));
}

// Get default datatype definitions
void FSLSemanticMap::GetDefaultDatatypeDefinitions(TArray<FNode>& OutNodes)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName OwlDP("owl", "DatatypeProperty");

	// Add comment to first node
	FNode NodeWithComment = FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "depthOfObject")));
	NodeWithComment.SetComment("Datatype Definitions");
	OutNodes.Add(NodeWithComment);

	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "heightOfObject"))));
	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "heightOfObject"))));
	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "widthOfObject"))));
	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "vectorX"))));
	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "vectorY"))));
	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "vectorZ"))));
	OutNodes.Add(FNode(OwlDP, FAttribute(RdfAbout, FAttributeValue("knowrob", "pathToCadModel"))));
}

// Get default class definitions
void FSLSemanticMap::GetDefaultClassDefinitions(TArray<FNode>& OutNodes)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName OwlClass("owl", "Class");

	// Add comment to first node
	FNode NodeWithComment = FNode(OwlClass, FAttribute(RdfAbout, FAttributeValue("knowrob", "SemanticEnvironmentMap")));
	NodeWithComment.SetComment("Class Definitions");
	OutNodes.Add(NodeWithComment);

	OutNodes.Add(FNode(OwlClass, FAttribute(RdfAbout, FAttributeValue("knowrob", "SLMapPerception"))));
	OutNodes.Add(FNode(OwlClass, FAttribute(RdfAbout, FAttributeValue("knowrob", "TimePoint"))));
	OutNodes.Add(FNode(OwlClass, FAttribute(RdfAbout, FAttributeValue("knowrob", "Vector"))));
}

// Add semantic map entries
void FSLSemanticMap::AddEntries(UWorld* World)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfType("rdf", "type");
	const FPrefixName KbPose("knowrob", "pose");
	const FPrefixName KbQuat("knowrob", "quaternion");
	const FPrefixName KbTransl("knowrob", "translation");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName RdfDatatype("rdf", "datatype");
	const FPrefixName OwlNI("owl", "NamedIndividual");

	// Iterate objects with SemLog tag key
	auto ObjToTagsMap = FTags::GetObjectsToKeyValuePairs(World, "SemLog");
	for (const auto& ObjToTagsItr : ObjToTagsMap)
	{
		// Take into account only objects with an id and class value set
		if (ObjToTagsItr.Value.Contains("Id") && ObjToTagsItr.Value.Contains("Class"))
		{
			const FString Id = ObjToTagsItr.Value["Id"];
			const FString Class = ObjToTagsItr.Value["Class"];

			if (AActor* ActEntry = Cast<AActor>(ObjToTagsItr.Key))
			{

				AddActorEntry(ActEntry, Id, Class);

				if (AActor* AttachedParent = ActEntry->GetAttachParentActor())
				{
					if (ObjToTagsMap.Contains(AttachedParent) &&
						ObjToTagsMap[AttachedParent].Contains("Id") &&
						ObjToTagsMap[AttachedParent].Contains("Class"))
					{

					}
				}
			}
			else if (USceneComponent* CompEntry = Cast<USceneComponent>(ObjToTagsItr.Key))
			{
				AddComponentEntry(CompEntry, Id, Class);
			}
		}
	}
}

// Add actor entry
void FSLSemanticMap::AddActorEntry(AActor* Actor, const FString& Id, const FString& Class)
{
	// Prefix name constants
	const FPrefixName RdfAbout("rdf", "about");
	const FPrefixName RdfType("rdf", "type");
	const FPrefixName KbPose("knowrob", "pose");
	const FPrefixName KbQuat("knowrob", "quaternion");
	const FPrefixName KbTransl("knowrob", "translation");
	const FPrefixName RdfResource("rdf", "resource");
	const FPrefixName RdfDatatype("rdf", "datatype");
	const FPrefixName OwlNI("owl", "NamedIndividual");
	
	// Attribute values constants
	const FAttributeValue AttrValPose("knowrob", "Pose");
	const FAttributeValue AttrValString("xsd", "string");
	
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(Actor->GetActorLocation());
	const FQuat ROSQuat = FConversions::UToROS(Actor->GetActorQuat());

	// Entity individual node
	FNode Individual(OwlNI, FAttribute(RdfAbout, FAttributeValue("log", Id)));
	Individual.SetComment("Object " + Class + " " + Id);
	FNode EntryClass(RdfType, FAttribute(RdfResource, FAttributeValue("knowrob", Class)));
	Individual.AddChildNode(EntryClass);
	const FString PoseId = FIds::NewGuidInBase64Url();
	FNode EntryPose(KbPose, FAttribute(RdfResource, FAttributeValue("log", PoseId)));
	Individual.AddChildNode(EntryPose);

	SemMap.GetRoot().AddChildNode(Individual);

	// Pose individual
	FNode PoseIndividual(OwlNI, FAttribute(RdfAbout, FAttributeValue("log", PoseId)));
	FNode PoseClass(RdfType, FAttribute(RdfResource, AttrValPose));
	PoseIndividual.AddChildNode(PoseClass);
	FNode PoseQuat(KbQuat, FAttribute(RdfDatatype, AttrValString), ROSQuat.ToString());
	PoseIndividual.AddChildNode(PoseQuat);
	FNode PoseTrans(KbTransl, FAttribute(RdfDatatype, AttrValString), ROSLoc.ToString());
	PoseIndividual.AddChildNode(PoseQuat);
	
	SemMap.GetRoot().AddChildNode(PoseIndividual);
}

// Add scene component entry
void FSLSemanticMap::AddComponentEntry(USceneComponent* Component, const FString& Id, const FString& Class)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(Component->GetComponentLocation());
	const FQuat ROSQuat = FConversions::UToROS(Component->GetComponentQuat());
}

// Write semantic map to file
bool FSLSemanticMap::WriteToFile(const FString& Filename)
{
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/") + Filename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(SemMap.ToString(), *FullFilePath);
}