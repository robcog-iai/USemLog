// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlEvents.h"

/**
* 
*/
struct FOwlEventsIAI : public FOwlEvents
{
public:
	// Default constructor
	FOwlEventsIAI() 
	{
		AddDefaultValues();
	}

protected:
	// Add default values to the document
	void AddDefaultValues()
	{
		AddEntityDefinitions();
		AddNamespaces();
		AddImports();
		AddPropertyDefinitions();
		AddDatatypeDefinitions();
		AddClassDefinitions();
	}

	// Add entity definitions
	void AddEntityDefinitions()
	{
		// DOCTYPE root name
		EntityDefinitions.Name = FOwlPrefixName("rdf", "RDF");
		// Declarations
		EntityDefinitions.EntityPairs.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
		EntityDefinitions.EntityPairs.Add("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
		EntityDefinitions.EntityPairs.Add("owl", "http://www.w3.org/2002/07/owl#");
		EntityDefinitions.EntityPairs.Add("xsd", "http://www.w3.org/2001/XMLSchema#");
		EntityDefinitions.EntityPairs.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	}

	// Add default owl namespace declarations
	void AddNamespaces()
	{
		Namespaces.Add(FOwlAttribute(FOwlPrefixName("xmlns", "computable"),
			FOwlAttributeValue("http://knowrob.org/kb/computable.owl#")));
		Namespaces.Add(FOwlAttribute(FOwlPrefixName("xmlns", "swrl"),
			FOwlAttributeValue("http://www.w3.org/2003/11/swrl#")));
		Namespaces.Add(FOwlAttribute(FOwlPrefixName("xmlns", "rdf"),
			FOwlAttributeValue("http://www.w3.org/1999/02/22-rdf-syntax-ns#")));
		Namespaces.Add(FOwlAttribute(FOwlPrefixName("xmlns", "owl"),
			FOwlAttributeValue("http://www.w3.org/2002/07/owl#")));
		// TODO contd.
	}
	
	// Add imports
	void AddImports()
	{
		// Prefix name constants
		const FOwlPrefixName OwlImports("owl", "imports");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfResource("rdf", "resource");

		// Create ontology import node
		OntologyImports.Name = FOwlPrefixName("owl", "Ontology");
		OntologyImports.Attributes.Add(FOwlAttribute(RdfAbout, FOwlAttributeValue("http://knowrob.org/kb/u_map.owl")));
		OntologyImports.Comment = TEXT("Ontologies");

		// Add child import nodes
		OntologyImports.ChildNodes.Add(FOwlNode(OwlImports,
			FOwlAttribute(RdfResource, FOwlAttributeValue("package://knowrob_common/owl/knowrob.owl"))));
		OntologyImports.ChildNodes.Add(FOwlNode(OwlImports,
			FOwlAttribute(RdfResource, FOwlAttributeValue("package://knowrob_common/owl/knowrob_u.owl"))));

	};

	// Add property definitions
	void AddPropertyDefinitions()
	{
		// Prefix name constants
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlOP("owl", "ObjectProperty");

		// Add comment to first node
		FOwlNode NodeWithComment = FOwlNode(OwlOP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "describedInMap")));
		NodeWithComment.Comment = TEXT("Property Definitions");
		PropertyDefinitions.Add(NodeWithComment);

		PropertyDefinitions.Add(FOwlNode(OwlOP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "describedInMap2"))));
	}

	// Add datatype definitions
	void  AddDatatypeDefinitions()
	{
		// Prefix name constants
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlDP("owl", "DatatypeProperty");

		// Add comment to first node
		FOwlNode NodeWithComment = FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "depthOfObject")));
		NodeWithComment.Comment = TEXT("Datatype Definitions");
		DatatypeDefinitions.Add(NodeWithComment);

		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "heightOfObject"))));
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "heightOfObject"))));
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "widthOfObject"))));
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "vectorX"))));
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "vectorY"))));
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "vectorZ"))));
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "pathToCadModel"))));

	}

	// Add class definitions
	void AddClassDefinitions()
	{
		// Prefix name constants
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlClass("owl", "Class");

		// Add comment to first node
		FOwlNode NodeWithComment = FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "SemanticEnvironmentMap")));
		NodeWithComment.Comment = TEXT("Class Definitions");
		ClassDefinitions.Add(NodeWithComment);

		ClassDefinitions.Add(FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "SLMapPerception"))));
		ClassDefinitions.Add(FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "TimePoint"))));
		ClassDefinitions.Add(FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "Vector"))));
	}
};
