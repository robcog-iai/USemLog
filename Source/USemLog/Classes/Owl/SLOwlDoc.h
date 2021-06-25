// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SLOwlNode.h"

/**
* OWL document
*/
struct FSLOwlDoc
{
public:
	// Document Type Definition (DTD) for Entity Declaration
	FSLOwlEntityDTD EntityDefinitions;

	// Namespaces
	TArray<FSLOwlAttribute> Namespaces;

	// Ontology imports 
	FSLOwlNode OntologyImports;

	// Property definitions
	TArray<FSLOwlNode> PropertyDefinitions;

	// Datatype definitions
	TArray<FSLOwlNode> DatatypeDefinitions;

	// Class definitions
	TArray<FSLOwlNode> ClassDefinitions;

	// Document node individuals
	TArray<FSLOwlNode> Individuals;

	// Prefix (e.g. &log from rdf:about="&log;abc123">")
	FString Prefix;
	
	// Ontology name (e.g UE-Experiment, from "http://knowrob.org/kb/UE-Experiment.owl#")
	FString OntologyName;
	
	// Id of the document
	FString Id;	

public:
	// Default constructor
	FSLOwlDoc() {}

	// Doc init constructor
	FSLOwlDoc(const FString& InPrefix,
		const FString& InOntologyName,
		const FString& InId) :
		Prefix(InPrefix),
		OntologyName(InOntologyName),
		Id(InId)
	{}
	
	// Destructor
	virtual ~FSLOwlDoc() {}

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
	void AddNamespaceDeclaration(const FSLOwlAttribute& InAttribute)
	{
		Namespaces.Add(InAttribute);
	}

	// Add namespace declaration
	void AddNamespaceDeclaration(const FSLOwlPrefixName& InPrefix,
		const FSLOwlAttributeValue& InAttributeValue)
	{
		AddNamespaceDeclaration(FSLOwlAttribute(InPrefix, InAttributeValue));
	}

	// Add namespace declaration
	void AddNamespaceDeclaration(const FString& InPrefix, const FString& InPrefixName,
		const FString& InAttributeValue)
	{
		AddNamespaceDeclaration(FSLOwlPrefixName(InPrefix, InPrefixName),
			FSLOwlAttributeValue(InAttributeValue));
	}

	// Add namespace declarations
	void AddNamespaceDeclarations(const TArray<FSLOwlAttribute>& InAttributes)
	{
		Namespaces.Append(InAttributes);
	}
	
	// Create ontology imports node
	void CreateOntologyNode()
	{
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName OwlOntology("owl", "Ontology");

		// Create ontology import node
		OntologyImports.Name = OwlOntology;
		OntologyImports.AddAttribute(FSLOwlAttribute(RdfAbout,
			FSLOwlAttributeValue("http://knowrob.org/kb/" + OntologyName + ".owl")));
		OntologyImports.Comment = TEXT("Ontologies:");
	}

	// Add ontology import child node
	void AddOntologyImport(const FString& Import)
	{
		const FSLOwlPrefixName OwlImports("owl", "imports");
		const FSLOwlPrefixName RdfResource("rdf", "resource");

		OntologyImports.AddChildNode(FSLOwlNode(OwlImports,
			FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(Import))));
	}

	// Add property definition
	void AddPropertyDefinition(const FString& InNs, const FString& InName)
	{
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName OwlOP("owl", "ObjectProperty");
		AddPropertyDefinition(FSLOwlNode(OwlOP, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InNs, InName))));
	}

	// Add property definition
	void AddPropertyDefinition(const FSLOwlNode& InNode)
	{
		PropertyDefinitions.Add(InNode);
	}

	// Add datatype definition
	void AddDatatypeDefinition(const FString& InNs, const FString& InName)
	{
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName OwlDP("owl", "DatatypeProperty");
		AddDatatypeDefinition(FSLOwlNode(OwlDP, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InNs, InName))));
	}

	// Add datatype definition
	void AddDatatypeDefinition(const FSLOwlNode& InNode)
	{
		DatatypeDefinitions.Add(InNode);
	}

	// Add class definition from namespace and value
	void AddClassDefinition(const FString& InNs, const FString& InName)
	{
		AddClassDefinition(FSLOwlAttributeValue(InNs, InName));
	}

	// Add class definition from attribute valu
	void AddClassDefinition(const FSLOwlAttributeValue& AV)
	{
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName OwlClass("owl", "Class");
		AddClassDefinition(FSLOwlNode(OwlClass, FSLOwlAttribute(RdfAbout, AV)));
	}

	// Add class definition node
	void AddClassDefinition(const FSLOwlNode& InNode)
	{
		ClassDefinitions.Add(InNode);
	}
	
	// Add individual node to the document
	void AddIndividual(const FSLOwlNode& InChildNode)
	{
		Individuals.Add(InChildNode);
	}

	// Add individuals to the document
	void AddIndividuals(const TArray<FSLOwlNode>& InChildNodes)
	{
		Individuals.Append(InChildNodes);
	}

	// Return document as string
	FString ToString() const
	{
		FString Indent = "";
		FString DocStr = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
		DocStr += EntityDefinitions.ToString();
		FSLOwlNode Root(FSLOwlPrefixName("rdf", "RDF"), Namespaces);
		Root.AddChildNode(OntologyImports);
		Root.AddChildNodes(PropertyDefinitions);
		Root.AddChildNodes(DatatypeDefinitions);
		Root.AddChildNodes(ClassDefinitions);
		Root.AddChildNodes(Individuals);
		DocStr += Root.ToString(Indent);
		return DocStr;
	}
};
