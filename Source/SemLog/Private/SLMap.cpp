// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMap.h"
#include "SLUtils.h"
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
		USLMap::InsertActorIndividual(ActorToTagItr);
	}

	// Get the map of components to their tag properties
	const TMap<UActorComponent*, TMap<FString, FString>> ComponentToTagProperties =
		FTagStatics::GetComponentsToKeyValuePairs(World, "SemLog");

	// Iterate all correctly tagged actors and add them to the semantic map
	for (const auto& ComponentToTagItr : ComponentToTagProperties)
	{
		USLMap::InsertComponentIndividual(ComponentToTagItr);
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
	// Set as FOwlObject
	SemMapIndividual.Set("u-map", "SemanticEnvironmentMap", FSLUtils::GenerateRandomFString(4));

	// Remove previous default attributes
	OwlDocument.DoctypeAttributes.Empty();
	OwlDocument.RdfAttributes.Empty();

	// Default doctype attributes for the semantic map
	OwlDocument.DoctypeAttributes.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns");
	OwlDocument.DoctypeAttributes.Add("rdfs", "http://www.w3.org/2000/01/rdf-schema");
	OwlDocument.DoctypeAttributes.Add("owl", "http://www.w3.org/2002/07/owl");
	OwlDocument.DoctypeAttributes.Add("xsd", "http://www.w3.org/2001/XMLSchema#");
	OwlDocument.DoctypeAttributes.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	OwlDocument.DoctypeAttributes.Add("knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	OwlDocument.DoctypeAttributes.Add("log", "http://knowrob.org/kb/unreal_log.owl#");
	OwlDocument.DoctypeAttributes.Add("u-map", "http://knowrob.org/kb/u_map.owl#");

	// Default rdf attributes for the semantic map
	OwlDocument.RdfAttributes.Add("xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:swrl", "http://www.w3.org/2003/11/swrl#");
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
		"owl:imports", "rdf:resource", "package://knowrob_robcog/owl/knowrob_u.owl"));
	TSharedPtr<FOwlNode> OntologyOwlNode = MakeShareable(new FOwlNode("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl",
		OntologyOwlProperties,
		"Ontologies"));
	OwlDocument.Nodes.Insert(OntologyOwlNode, 0);

	// Add object property definitions 
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;describedInMap",
		"Property Definitions")));

	// Add datatype property definitions
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;depthOfObject",
		"Datatype Definitions")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;heightOfObject")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;widthOfObject")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorX")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorY")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;vectorZ")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:DatatypeProperty", "rdf:about", "&knowrob;pathToCadModel")));

	// Add class definitions
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob;SemanticEnvironmentMap",
		"Class Definitions")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob;SLMapPerception")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob;TimePoint")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob;Vector")));
		
	// Add semantic map individual
	TArray<FOwlTriple> SemMapOwlProperties;
	SemMapOwlProperties.Emplace(
		FOwlTriple("rdf:type", "rdf:resource", "&knowrob;SemanticEnvironmentMap"));

	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:NamedIndividual", "rdf:about", SemMapIndividual.GetFullName(),
		SemMapOwlProperties,
		"Semantic Environment Map")));

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

// Insert actor individual to the map with its 3D transform
void USLMap::InsertActorIndividual(const TPair<AActor*, TMap<FString, FString>>& ActorWithProperties)
{
	const FString IndividualClass = ActorWithProperties.Value.Contains("Class")
		? *ActorWithProperties.Value.Find("Class") : FString("DefaultClass");
	const FString IndividualId = ActorWithProperties.Value.Contains("Id")
		? *ActorWithProperties.Value.Find("Id") : FString("DefaultId");

	const FVector Loc = ActorWithProperties.Key->GetActorLocation();
	const FQuat Quat = ActorWithProperties.Key->GetActorQuat();
	const FVector Box = ActorWithProperties.Key->GetComponentsBoundingBox().GetSize();

	// Add to map
	USLMap::InsertIndividual(IndividualClass, IndividualId, Loc, Quat, Box);
}

// Insert individual to the map with its 3D transform
void USLMap::InsertComponentIndividual(const TPair<UActorComponent*, TMap<FString, FString>>& ComponentWithProperties)
{
	if (ComponentWithProperties.Key->IsA(USceneComponent::StaticClass()))
	{
		USceneComponent* SceneComp = Cast<USceneComponent>(ComponentWithProperties.Key);

		const FString IndividualClass = ComponentWithProperties.Value.Contains("Class")
			? *ComponentWithProperties.Value.Find("Class") : FString("DefaultClass");
		const FString IndividualId = ComponentWithProperties.Value.Contains("Id")
			? *ComponentWithProperties.Value.Find("Id") : FString("DefaultId");

		const FVector Loc = SceneComp->GetComponentLocation();
		const FQuat Quat = SceneComp->GetComponentQuat();
		const FVector Box = SceneComp->Bounds.BoxExtent;		

		// Add to map
		USLMap::InsertIndividual(IndividualClass, IndividualId, Loc, Quat, Box);
	}
}

// Insert individual to the document
void USLMap::InsertIndividual(
	const FString IndividualClass,
	const FString IndividualId,
	const FVector Location,
	const FQuat Quat,
	const FVector BoundingBox,
	bool bSaveAsRightHandedCoordinate)
{
	const FString IndividualName = IndividualClass + "_" + IndividualId;
	const FString PerceptionId = FSLUtils::GenerateRandomFString(4);
	const FString TransfId = FSLUtils::GenerateRandomFString(4);

	// Get location as string in right(ROS) or left (UE4) hand coordinate system
	const FString LocStr = 	bSaveAsRightHandedCoordinate ?
		(FString::SanitizeFloat(Location.X / 100.f) + " "
			+ FString::SanitizeFloat(-Location.Y / 100.f) + " "
			+ FString::SanitizeFloat(Location.Z / 100.f))
		:
		(FString::SanitizeFloat(Location.X / 100.f) + " "
			+ FString::SanitizeFloat(Location.Y / 100.f) + " "
			+ FString::SanitizeFloat(Location.Z / 100.f));

	// Get orientation as string in right(ROS) or left (UE4) hand coordinate system
	const FString QuatStr = bSaveAsRightHandedCoordinate ?
		(FString::SanitizeFloat(Quat.W) + " "
			+ FString::SanitizeFloat(-Quat.X) + " "
			+ FString::SanitizeFloat(Quat.Y) + " "
			+ FString::SanitizeFloat(-Quat.Z))
		:
		(FString::SanitizeFloat(Quat.W) + " "
			+ FString::SanitizeFloat(Quat.X) + " "
			+ FString::SanitizeFloat(Quat.Y) + " "
			+ FString::SanitizeFloat(Quat.Z));

	// Add object individual
	TArray<FOwlTriple> IndividualProperties;
	IndividualProperties.Emplace(FOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;" + IndividualClass));
	IndividualProperties.Emplace(FOwlTriple("knowrob:depthOfObject", "rdf:datatype",
		"&xsd;double", FString::SanitizeFloat(BoundingBox.X / 100.f)));
	IndividualProperties.Emplace(FOwlTriple("knowrob:widthOfObject", "rdf:datatype",
		"&xsd;double", FString::SanitizeFloat(BoundingBox.Y / 100.f)));
	IndividualProperties.Emplace(FOwlTriple("knowrob:heightOfObject", "rdf:datatype",
		"&xsd;double", FString::SanitizeFloat(BoundingBox.Z / 100.f)));
	IndividualProperties.Emplace(FOwlTriple("knowrob:pathToCadModel", "rdf:datatype",
		"&xsd;string", "package://robcog/" + IndividualClass + ".dae"));
	IndividualProperties.Emplace(FOwlTriple(
		"knowrob:describedInMap", "rdf:resource", SemMapIndividual.GetFullName()));

	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:NamedIndividual", "rdf:about", "&log;" + IndividualName,
		IndividualProperties,
		"Object " + IndividualName)));

	// Add perception event for localization
	TArray<FOwlTriple> PerceptionProperties;
	PerceptionProperties.Emplace(FOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;SemanticMapPerception"));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:eventOccursAt", "rdf:resource", "&u-map;Transformation_" + TransfId));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:startTime", "rdf:resource", "&u-map;timepoint_0"));
	PerceptionProperties.Emplace(FOwlTriple(
		"knowrob:objectActedOn", "rdf:resource", "&log;" + IndividualName));

	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:NamedIndividual", "rdf:about", "&u-map;SemanticMapPerception_" + PerceptionId,
		PerceptionProperties)));

	// Add transform for the perception event
	TArray<FOwlTriple> TransfProperties;
	TransfProperties.Emplace(FOwlTriple(
		"rdf:type", "rdf:resource", "&knowrob;Transformation"));
	TransfProperties.Emplace(FOwlTriple(
		"knowrob:quaternion", "rdf:datatype", "&xsd;string", QuatStr));
	TransfProperties.Emplace(FOwlTriple(
		"knowrob:translation", "rdf:datatype", "&xsd;string", LocStr));

	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:NamedIndividual", "rdf:about", "&u-map;Transformation_" + TransfId,
		TransfProperties)));
}