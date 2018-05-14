// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Owl.h"
#include "Owl/Node.h"
#include "Owl/Doc.h"

namespace SLOwl
{
	/**
	 * 
	 */
	struct FSemanticMap
	{
	public:
		// Default constructor
		FSemanticMap() {}

		// Init constructor
		FSemanticMap(const FEntityDTD& InEntityDefinitions,
			const TArray<SLOwl::FAttribute>& InNamespaces,
			const SLOwl::FNode& InOntologyImports,
			const TArray<SLOwl::FNode>& InPropertyDefinitions,
			const TArray<SLOwl::FNode>& InDatatypeDefinitions,
			const TArray<SLOwl::FNode>& InClassDefinitions,
			TArray<SLOwl::FNode>& InEntries)) :
			EntityDefinitions(InEntityDefinitions),
			Namespaces(InNamespaces),
			OntologyImports(InOntologyImports),
			PropertyDefinitions(InPropertyDefinitions),
			DatatypeDefinitions(InDatatypeDefinitions),
			ClassDefinitions(InClassDefinitions),
			Entries(InEntries)
		{}

		// Destructor
		~FSemanticMap() {}

		// Return semantic map as owl document
		FDoc ToDoc()
		{
			FNode Root(FPrefixName("rdf", "RDF"), Namespaces);
			Root.ChildNodes.Add(OntologyImports);
			Root.ChildNodes.Append(PropertyDefinitions);
			Root.ChildNodes.Append(DatatypeDefinitions);
			Root.ChildNodes.Append(ClassDefinitions);
			Root.ChildNodes.Append(Entries);
			const FString Declaration = Declaration = 
				TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
			return FDoc(Declaration, EntityDefinitions, Root);
		}

		// Return semantic map as string
		FString ToString()
		{
			return ToDoc().ToString();
		}

		// Entity definitions
		SLOwl::FEntityDTD EntityDefinitions; 

		// Namespace declarations
		TArray<SLOwl::FAttribute> Namespaces;

		// Ontology imports 
		SLOwl::FNode OntologyImports;

		// Property definitions
		TArray<SLOwl::FNode> PropertyDefinitions;

		// Datatype definitions
		TArray<SLOwl::FNode> DatatypeDefinitions;

		// Class definitions
		TArray<SLOwl::FNode> ClassDefinitions;

		// Entity entries
		TArray<SLOwl::FNode> Entries;
	};

} // namespace SLOwl