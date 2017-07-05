// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMap.h"
#include "SLStatics.h"
#include "TagStatics.h"
#include "PlatformFilemanager.h"
#include "FileManager.h"
#include "FileHelper.h"

// Constructor, set default values
USLMap::USLMap()
{
	// Default values
	bOwlDefaultValuesSet = false;

	// Default filename
	Filename = "SemanticMap.owl";

	// Default log folder path
	LogDirectoryPath = FPaths::GameDir() + "SemLog";

	// Check if the semantic map already exists or not
	bExists = USLMap::Exists();

	// If map does not exists, set default values
	if (!bExists)
	{
		USLMap::SetDefaultValues();
	}
}

// Destructor
USLMap::~USLMap()
{
}

// Check if document exists
bool USLMap::Exists()
{
	const FString FilePath = LogDirectoryPath.EndsWith("/") ?
		LogDirectoryPath + Filename : LogDirectoryPath + "/" + Filename;
	return IFileManager::Get().FileExists(*FilePath);

}

// Generate the semantic map
bool USLMap::Generate(UWorld* World)
{	
	// Set default values
	if (!bOwlDefaultValuesSet)
	{
		USLMap::SetDefaultValues();
	}

	// Get the map of actors to their tag properties
	const TMap<AActor*, TMap<FString, FString>> ActorToTagProperties =
		FTagStatics::GetActorsToKeyValuePairs(World, "SemLog");


	// Iterate all correctly tagged actors and add them to the semantic map
	for (const auto& ActorToTagItr : ActorToTagProperties)
	{
		USLMap::InsertIndividual(ActorToTagItr);
	}
	
	return true;
}

// Write document to file
bool USLMap::WriteToFile(bool bOverwrite)
{
	const FString FilePath = LogDirectoryPath.EndsWith("/") ?
		LogDirectoryPath + Filename : LogDirectoryPath + "/" + Filename;

	// Return if file already exists and it should not be rewritten
	if (IFileManager::Get().FileExists(*FilePath) && !bOverwrite)
	{
		UE_LOG(LogTemp, Error, TEXT("%s already exists at %s, and it should not be rewritten!"),
			*Filename, *LogDirectoryPath);
		return false;
	}

	// Creates directory tree as well
	return FFileHelper::SaveStringToFile(OwlDocument.ToXmlString(), *FilePath);
}

// Set document default values
void USLMap::SetDefaultValues()
{
	// 
	Class = "SemanticEnvironmentMap";
	Id = FSLStatics::GenerateRandomFString(4);
	Name = Class + "_" + Id;
	Ns = "u-map";
	FullName = "&" + Ns + ";" + Name;

	// Set as FOwlObject
	OwlObject.Set(FullName);

	// Remove previous default attributes
	OwlDocument.DoctypeAttributes.Empty();
	OwlDocument.RdfAttributes.Empty();

	// Default doctype attributes for the semantic map
	OwlDocument.DoctypeAttributes.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns");
	OwlDocument.DoctypeAttributes.Add("rdfs", "http://www.w3.org/2002/07/owl");
	OwlDocument.DoctypeAttributes.Add("owl", "http://www.w3.org/2001/XMLSchema#");
	OwlDocument.DoctypeAttributes.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	OwlDocument.DoctypeAttributes.Add("knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	OwlDocument.DoctypeAttributes.Add("log", "http://knowrob.org/kb/unreal_log.owl#");
	OwlDocument.DoctypeAttributes.Add("u-map", "http://knowrob.org/kb/u_map.owl#");

	// Default rdf attributes for the semantic map
	OwlDocument.RdfAttributes.Add("xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	OwlDocument.RdfAttributes.Add("xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	OwlDocument.RdfAttributes.Add("xmlns:owl", "http://www.w3.org/2002/07/owl#");
	OwlDocument.RdfAttributes.Add("xmlns:knowrob", "http://knowrob.org/kb/knowrob.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:u-map", "http://knowrob.org/kb/u_map.owl#");
	OwlDocument.RdfAttributes.Add("xml:base", "http://knowrob.org/kb/u_map.owl#");

	// Create and insert the ontologies node at the beginning
	TArray<FOwlTriple> OntologyOwlProperties;
	OntologyOwlProperties.Emplace(FOwlTriple(
		"owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	OntologyOwlProperties.Emplace(FOwlTriple(
		"owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob_u.owl"));
	FOwlNode OntologyOwlNode("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl",
		OntologyOwlProperties,
		"Ontologies");
	OwlDocument.Nodes.Insert(OntologyOwlNode, 0);

	// Add object property definitions 
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;describedInMap",
		"Property Definitions"));

	// Add datatype property definitions
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;depthOfObject",
		"Datatype Definitions"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;heightOfObject"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about",	"&knowrob;widthOfObject"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorX"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about",	"&knowrob;vectorY"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorZ"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:DatatypeProperty", "rdf:about",	"&knowrob;pathToCadModel"));

	// Add class definitions
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob;SemanticEnvironmentMap",
		"Class Definitions"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob;SLMapPerception"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class",	"rdf:about", "&knowrob;TimePoint"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class",	"rdf:about", "&knowrob;Vector"));
		
	// Add semantic map individual
	TArray<FOwlTriple> SemMapOwlProperties;
	SemMapOwlProperties.Emplace(
		FOwlTriple("rdf:type", "rdf:resource", "&knowrob;SemanticEnvironmentMap"));

	OwlDocument.Nodes.Emplace(FOwlNode("owl:NamedIndividual", "rdf:about", FullName,
		SemMapOwlProperties,
		"Semantic Environment Map"));

	// Mark that default values have been set
	bOwlDefaultValuesSet = true;
}

// Remove document default values
void USLMap::RemoveDefaultValues()
{
	// Remove default attributes
	OwlDocument.DoctypeAttributes.Empty();
	OwlDocument.RdfAttributes.Empty();
	
	// Mark that default values have been removed
	bOwlDefaultValuesSet = false;
}

// Insert individual to the map with its 3D transform
bool USLMap::InsertIndividual(const TPair<AActor*, TMap<FString, FString>>& ActorWithProperties)
{
	const FString IndividualClass = ActorWithProperties.Value.Contains("Class")
		? *ActorWithProperties.Value.Find("Class") : FString("DefaultClass");
	const FString IndividualId = ActorWithProperties.Value.Contains("Id")
		? *ActorWithProperties.Value.Find("Id") : FString("DefaultId");
	const FString IndividualName = IndividualClass + "_" + IndividualId;
	const FString PerceptionId = FSLStatics::GenerateRandomFString(4);
	const FString TransfId = FSLStatics::GenerateRandomFString(4);

	const FVector Loc = ActorWithProperties.Key->GetActorLocation();
	const FQuat Quat = ActorWithProperties.Key->GetActorQuat();
	const FVector Box = ActorWithProperties.Key->GetComponentsBoundingBox().GetSize() / 100;

	const FString LocStr = FString::SanitizeFloat(Loc.X) + " "
		+ FString::SanitizeFloat(-Loc.Y) + " "
		+ FString::SanitizeFloat(Loc.Z);
	const FString QuatStr = FString::SanitizeFloat(Quat.W) + " "
		+ FString::SanitizeFloat(-Quat.X) + " "
		+ FString::SanitizeFloat(Quat.Y) + " "
		+ FString::SanitizeFloat(-Quat.Z);
	
	// Add object individual
	TArray<FOwlTriple> IndividualProperties;
	IndividualProperties.Emplace(FOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;" + IndividualClass));
	IndividualProperties.Emplace(FOwlTriple(
		"knowrob:depthOfObject", "rdf:datatype", "&xsd;double", FString::SanitizeFloat(Box.X)));
	IndividualProperties.Emplace(FOwlTriple(
		"knowrob:widthOfObject", "rdf:datatype", "&xsd;double", FString::SanitizeFloat(Box.Y)));
	IndividualProperties.Emplace(FOwlTriple(
		"knowrob:heightOfObject", "rdf:datatype", "&xsd;double", FString::SanitizeFloat(Box.Z)));
	IndividualProperties.Emplace(FOwlTriple(
		"knowrob:pathToCadModel", "rdf:datatype", "&xsd;string", "package://robcog/" + IndividualClass + ".dae"));
	IndividualProperties.Emplace(FOwlTriple(
		"knowrob:describedInMap", "rdf:resource", FullName));

	OwlDocument.Nodes.Emplace(FOwlNode(
		"owl:NamedIndividual",
		"rdf:about=",
		"&log;" + IndividualName,
		IndividualProperties,
		"Object " + IndividualName));
	
	// Add perception event for localization
	TArray<FOwlTriple> PerceptionProperties;
	PerceptionProperties.Emplace(FOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;SLMapPerception"));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:eventOccursAt", "rdf:resource", "&u-map;Transformation_" + TransfId));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:startTime", "rdf:resource", "&u-map;timepoint_0"));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:objectActedOn", "rdf:resource", "&log;" + IndividualName));

	OwlDocument.Nodes.Emplace(FOwlNode(
		"owl:NamedIndividual",
		"rdf:about",
		"&u-map;Transformation_" + TransfId,
		PerceptionProperties));

	// Add transform for the perception event
	TArray<FOwlTriple> TransfProperties;
	TransfProperties.Emplace(FOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;Transformation"));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:quaternion", "rdf:datatype", "&xsd;string", QuatStr));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:translation", "rdf:datatype", "&xsd;string", LocStr));

	OwlDocument.Nodes.Emplace(FOwlNode(
		"owl:NamedIndividual",
		"rdf:about",
		"&u-map;Transformation_" + TransfId,
		TransfProperties));

	return true;
}
