// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlDoc.h"

/**
* 
*/
struct FOwlSemanticMap
{
public:
	// Default constructor
	FOwlSemanticMap() {}

	// Init constructor
	FOwlSemanticMap(const FOwlEntityDTD& InEntityDefinitions,
		const TArray<FOwlAttribute>& InNamespaces,
		const FOwlNode& InOntologyImports,
		const TArray<FOwlNode>& InPropertyDefinitions,
		const TArray<FOwlNode>& InDatatypeDefinitions,
		const TArray<FOwlNode>& InClassDefinitions,
		TArray<FOwlNode>& InEntries) :
		EntityDefinitions(InEntityDefinitions),
		Namespaces(InNamespaces),
		OntologyImports(InOntologyImports),
		PropertyDefinitions(InPropertyDefinitions),
		DatatypeDefinitions(InDatatypeDefinitions),
		ClassDefinitions(InClassDefinitions),
		Entries(InEntries)
	{}

	// Destructor
	~FOwlSemanticMap() {}

	// Return semantic map as owl document
	FOwlDoc ToDoc()
	{
		FOwlNode Root(FOwlPrefixName("rdf", "RDF"), Namespaces);
		Root.ChildNodes.Add(OntologyImports);
		Root.ChildNodes.Append(PropertyDefinitions);
		Root.ChildNodes.Append(DatatypeDefinitions);
		Root.ChildNodes.Append(ClassDefinitions);
		Root.ChildNodes.Append(Entries);
		const FString Declaration =  
			TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		return FOwlDoc(Declaration, EntityDefinitions, Root);
	}

	// Return semantic map as string
	FString ToString()
	{
		return ToDoc().ToString();
	}

	// Entity definitions
	FOwlEntityDTD EntityDefinitions; 

	// Namespace declarations
	TArray<FOwlAttribute> Namespaces;

	// Ontology imports 
	FOwlNode OntologyImports;

	// Property definitions
	TArray<FOwlNode> PropertyDefinitions;

	// Datatype definitions
	TArray<FOwlNode> DatatypeDefinitions;

	// Class definitions
	TArray<FOwlNode> ClassDefinitions;

	// Entity entries
	TArray<FOwlNode> Entries;
};
