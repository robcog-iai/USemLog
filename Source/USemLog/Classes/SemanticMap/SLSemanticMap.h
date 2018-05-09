// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Owl.h"
#include "Owl/Doc.h"

/**
 * 
 */
class USEMLOG_API FSLSemanticMap
{
public:
	// Constructor
	FSLSemanticMap();

	// Constructor with map generation
	FSLSemanticMap(UWorld* World, const FString& InDirectory = TEXT("SemLog"));

	// Destructor
	~FSLSemanticMap();

	// Generate semantic map from world
	void Generate(UWorld* World);

	// Export semantic map to file
	bool WriteToFile(const FString& Filename = TEXT("SemanticMap"));

private:
	// Get default document type definitions
	void GetDefaultDTD(SLOwl::FEntityDTD& OutDTD);

	// Get default namespaces
	void GetDefaultNamespaces(TArray<SLOwl::FAttribute>& OutAttributes);

	// Get default ontology imports
	void GetDefaultOntologyImports(SLOwl::FNode& OutImports);

	// Get default property definitions
	void GetDefaultPropertyDefinitions(TArray<SLOwl::FNode>& OutNodes);

	// Get default datatype definitions
	void GetDefaultDatatypeDefinitions(TArray<SLOwl::FNode>& OutNodes);

	// Get default class definitions
	void GetDefaultClassDefinitions(TArray<SLOwl::FNode>& OutNodes);

	// Add semantic map entries
	void AddEntries(UWorld* World);

	// Add actor entry
	void AddActorEntry(AActor* Actor, const FString& Id, const FString& Class);

	// Add scene component entry
	void AddComponentEntry(USceneComponent* Component, const FString& Id, const FString& Class);

	// Path for saving the semantic map
	FString LogDirectory;

	// Semantic map as owl document
	SLOwl::FDoc SemMap;

	/** TODO **/
	// Entity definitions
	SLOwl::FEntityDTD EntityDefinitions; // TODO use namespace shortcuts

	// Namespace declarations
	TArray<SLOwl::FAttribute> Namespaces;

	// Ontology imports 
	FNode OntologyImports;

	// Property definitions
	TArray<SLOwl::FNode> PropertyDefinitions;

	// Datatype definitions
	TArray<SLOwl::FNode> DatatypeDefinitions;

	// Class definitions
	TArray<SLOwl::FNode> ClassDefinitions;

	// Entity entries
	TArray<SLOwl::FNode> Entries;
};
