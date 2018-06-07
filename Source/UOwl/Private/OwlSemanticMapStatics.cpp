// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "OwlSemanticMapStatics.h"

/* Semantic map template creation */
// Create Default semantic map
TSharedPtr<FOwlSemanticMap> FOwlSemanticMapStatics::CreateDefaultSemanticMap(
	const FString& InMapId,
	const FString& InMapPrefix,
	const FString& InMapName)
{
	// Create map document
	TSharedPtr<FOwlSemanticMap> SemMap = MakeShareable(new FOwlSemanticMap(InMapPrefix, InMapName, InMapId));

	// Add definitions
	SemMap->AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
	SemMap->AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
	SemMap->AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	SemMap->AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	SemMap->AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	SemMap->AddEntityDefintion(InMapPrefix, "http://knowrob.org/kb/" + InMapName + ".owl#");

	// Add namespaces
	SemMap->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InMapName + ".owl#");
	SemMap->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InMapName + ".owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	SemMap->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	SemMap->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	SemMap->AddNamespaceDeclaration("xmlns", InMapPrefix, "http://knowrob.org/kb/" + InMapName + ".owl#");

	// Set and add imports
	SemMap->SetOntologyNode(InMapName);
	SemMap->AddOntologyImport("package://knowrob_common/owl/knowrob.owl");

	// Add property definitions
	SemMap->AddPropertyDefinition("knowrob", "describedInMap");

	// Add datatype definitions
	SemMap->AddDatatypeDefinition("knowrob", "quaternion");
	SemMap->AddDatatypeDefinition("knowrob", "translation");
	
	// Add class definitions
	SemMap->AddClassDefinition("knowrob", "SemanticEnvironmentMap");
	SemMap->AddClassDefinition("knowrob", "Pose");

	return SemMap;
}

// Create IAI Kitchen semantic map
TSharedPtr<FOwlSemanticMap> FOwlSemanticMapStatics::CreateIAIKitchenSemanticMap(
	const FString& InMapId,
	const FString& InMapPrefix,
	const FString& InMapName)
{
	TSharedPtr<FOwlSemanticMap> SemMap = FOwlSemanticMapStatics::CreateDefaultSemanticMap(
		InMapId, InMapPrefix, InMapName);

	SemMap->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_kitchen_ue.owl");

	return SemMap;
}

// Create IAI Supermarket semantic map
TSharedPtr<FOwlSemanticMap> FOwlSemanticMapStatics::CreateIAISupermarketSemanticMap(
	const FString& InMapId,
	const FString& InMapPrefix,
	const FString& InMapName)
{
	TSharedPtr<FOwlSemanticMap> SemMap = FOwlSemanticMapStatics::CreateDefaultSemanticMap(
		InMapId, InMapPrefix, InMapName);

	SemMap->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_supermarket_ue.owl");

	return SemMap;
}