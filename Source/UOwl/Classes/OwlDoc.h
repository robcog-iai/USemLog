// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlNode.h"

/**
* OWL document
*/
struct FOwlDoc
{
public:
	// Document Type Definition (DTD) for Entity Declaration
	FOwlEntityDTD EntityDefinitions;

	// Namespaces
	TArray<FOwlAttribute> Namespaces;

	// Ontology imports 
	FOwlNode OntologyImports;

	// Property definitions
	TArray<FOwlNode> PropertyDefinitions;

	// Datatype definitions
	TArray<FOwlNode> DatatypeDefinitions;

	// Class definitions
	TArray<FOwlNode> ClassDefinitions;

	// Document node individuals
	TArray<FOwlNode> Individuals;

protected:
	// Current state of the indentation for writing to string
	FString Indent;

public:
	// Default constructor
	FOwlDoc() {}
	
	// Destructor
	virtual ~FOwlDoc() {}

	// Add entity definition
	void AddEntityDefintion(const TPairString& InEntityDefinition)
	{
		EntityDefinitions.AddPair(InEntityDefinition);
	}

	// Add entity definition
	void AddEntityDefintion(const FString& InKey, const FString& InVal)
	{
		EntityDefinitions.AddPair(TPairString(InKey, InVal));
	}

	// Add entity definitions
	void AddEntityDefintions(const TArray<TPairString>& InEntityDefinitions)
	{
		EntityDefinitions.AddPairs(InEntityDefinitions);
	}

	// Add namespace declaration
	void AddNamespaceDeclaration(const FOwlAttribute& InAttribute)
	{
		Namespaces.Add(InAttribute);
	}

	// Add namespace declaration
	void AddNamespaceDeclaration(const FOwlPrefixName& InPrefix,
		const FOwlAttributeValue& InAttributeValue)
	{
		AddNamespaceDeclaration(FOwlAttribute(InPrefix, InAttributeValue));
	}

	// Add namespace declaration
	void AddNamespaceDeclaration(const FString& InPrefix, const FString& InPrefixName,
		const FString& InAttributeValue)
	{
		AddNamespaceDeclaration(FOwlPrefixName(InPrefix, InPrefixName),
			FOwlAttributeValue(InAttributeValue));
	}

	// Add namespace declarations
	void AddNamespaceDeclarations(const TArray<FOwlAttribute>& InAttributes)
	{
		Namespaces.Append(InAttributes);
	}
	
	// Create ontology imports node
	void SetOntologyNode(const FString& InMapName)
	{
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlOntology("owl", "Ontology");

		// Create ontology import node
		OntologyImports.Name = OwlOntology;
		OntologyImports.AddAttribute(FOwlAttribute(RdfAbout,
			FOwlAttributeValue("http://knowrob.org/kb/" + InMapName + ".owl")));
		OntologyImports.Comment = TEXT("Ontologies");
	}

	// Add ontology import child node
	void AddOntologyImport(const FString& Import)
	{
		const FOwlPrefixName OwlImports("owl", "imports");
		const FOwlPrefixName RdfResource("rdf", "resource");

		OntologyImports.AddChildNode(FOwlNode(OwlImports,
			FOwlAttribute(RdfResource, FOwlAttributeValue(Import))));
	}

	// Add property definition
	void AddPropertyDefinition(const FString& InNs, const FString& InName)
	{
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlOP("owl", "ObjectProperty");
		AddPropertyDefinition(FOwlNode(OwlOP, FOwlAttribute(RdfAbout, FOwlAttributeValue(InNs, InName))));
	}

	// Add property definition
	void AddPropertyDefinition(const FOwlNode& InNode)
	{
		PropertyDefinitions.Add(InNode);
	}

	// Add datatype definition
	void AddDatatypeDefinition(const FString& InNs, const FString& InName)
	{
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlDP("owl", "DatatypeProperty");
		AddDatatypeDefinition(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue(InNs, InName))));
	}

	// Add datatype definition
	void AddDatatypeDefinition(const FOwlNode& InNode)
	{
		DatatypeDefinitions.Add(InNode);
	}

	// Add class definition
	void AddClassDefinition(const FString& InNs, const FString& InName)
	{
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlClass("owl", "Class");
		AddClassDefinition(FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue(InNs, InName))));
	}

	// Add class definition
	void AddClassDefinition(const FOwlNode& InNode)
	{
		ClassDefinitions.Add(InNode);
	}
	
	// Add individual node to the document
	void AddIndividual(const FOwlNode& InChildNode)
	{
		Individuals.Add(InChildNode);
	}

	// Add individuals to the document
	void AddIndividuals(const TArray<FOwlNode>& InChildNodes)
	{
		Individuals.Append(InChildNodes);
	}

	// Return document as string
	FString ToString()
	{
		FString DocStr = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
		DocStr += EntityDefinitions.ToString();
		FOwlNode Root(FOwlPrefixName("rdf", "RDF"), Namespaces);
		Root.AddChildNode(OntologyImports);
		Root.AddChildNodes(PropertyDefinitions);
		Root.AddChildNodes(DatatypeDefinitions);
		Root.AddChildNodes(ClassDefinitions);
		Root.AddChildNodes(Individuals);
		DocStr += Root.ToString(Indent);
		return DocStr;
	}
};
