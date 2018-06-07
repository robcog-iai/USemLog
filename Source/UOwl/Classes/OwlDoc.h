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

	// Document node entries
	TArray<FOwlNode> Entries;

protected:
	// Current state of the indentation for writing to string
	FString Indent;

public:
	// Default constructor
	FOwlDoc() {}

	//// Init constructor
	//FOwlDoc(const TArray<TPairString>& InEntityDefinitions,
	//	const TArray<FOwlAttribute>& InNamespaces,
	//	const FOwlNode& InOntologyImports,
	//	const TArray<FOwlNode>& InPropertyDefinitions,
	//	const TArray<FOwlNode>& InDatatypeDefinitions,
	//	const TArray<FOwlNode>& InClassDefinitions,
	//	const TArray<FOwlNode>& InEntries) :
	//	OntologyImports(InOntologyImports),
	//	PropertyDefinitions(InPropertyDefinitions),
	//	DatatypeDefinitions(InDatatypeDefinitions),
	//	ClassDefinitions(InClassDefinitions),
	//	Entries(InEntries)
	//{}
	
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

	// Add ontology import child nodes
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
		PropertyDefinitions.Add(FOwlNode(OwlOP, FOwlAttribute(RdfAbout, FOwlAttributeValue(InNs, InName))));
	}

	// Add datatype definition
	void AddDatatypeDefinition(const FString& InNs, const FString& InName)
	{
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlDP("owl", "DatatypeProperty");
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue(InNs, InName))));
	}

	// Add class definition
	void AddClassDefinition(const FString& InNs, const FString& InName)
	{
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlClass("owl", "Class");
		ClassDefinitions.Add(FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue(InNs, InName))));
	}
	
	// Add entry node to the document
	void AddEntry(const FOwlNode& InChildNode)
	{
		Entries.Add(InChildNode);
	}

	// Add entries to the document
	void AddEntries(const TArray<FOwlNode>& InChildNodes)
	{
		Entries.Append(InChildNodes);
	}

	// Return document as string
	virtual FString ToString()
	{
		FString DocStr = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
		DocStr += EntityDefinitions.ToString();
		FOwlNode Root(FOwlPrefixName("rdf", "RDF"), Namespaces);
		Root.AddChildNode(OntologyImports);
		Root.AddChildNodes(PropertyDefinitions);
		Root.AddChildNodes(DatatypeDefinitions);
		Root.AddChildNodes(ClassDefinitions);
		Root.AddChildNodes(Entries);
		DocStr += Root.ToString(Indent);
		return DocStr;
	}
};
