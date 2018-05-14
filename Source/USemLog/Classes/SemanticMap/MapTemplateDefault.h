#pragma once

#include "Owl/Owl.h"
#include "Owl/Doc.h"

/**
* Structure containing the default values for a semantic map
*/
struct FMapTemplateDefault
{
	// Constructor
	FMapTemplateDefault()
	{
		Build();
	}

	// Output constructor
	FMapTemplateDefault(FString& OutDeclaration,
		SLOwl::FEntityDTD& OutEntityDefinitions,
		TArray<SLOwl::FAttribute>& OutNamespaces,
		SLOwl::FNode& OutOntologyImports,
		TArray<SLOwl::FNode>& OutPropertyDefinitions,
		TArray<SLOwl::FNode>& OutDatatypeDefinitions,
		TArray<SLOwl::FNode>& OutClassDefinitions,
		TArray<SLOwl::FNode>& OutEntries)
	{
		Build();
		OutDeclaration = Declaration;
		OutEntityDefinitions = EntityDefinitions;
		OutNamespaces = Namespaces;
		OutOntologyImports = OntologyImports;
		OutPropertyDefinitions = PropertyDefinitions;
		OutDatatypeDefinitions = DatatypeDefinitions;
		OutClassDefinitions = ClassDefinitions;
		OutEntries = Entries;	
	}

	// Create map from the data
	SLOwl::FDoc ToDoc() const
	{
		SLOwl::FNode Root(SLOwl::FPrefixName("rdf", "RDF"), Namespaces);
		Root.ChildNodes.Add(OntologyImports);
		Root.ChildNodes.Append(PropertyDefinitions);
		Root.ChildNodes.Append(DatatypeDefinitions);
		Root.ChildNodes.Append(ClassDefinitions);
		Root.ChildNodes.Append(Entries);
		return SLOwl::FDoc(Declaration, EntityDefinitions, Root);
	}

	// XML Declaration
	FString Declaration;

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

protected:
	// Build template
	void Build()
	{
		SetDeclaration();
		SetEntityDefinitions();
		SetNamespaces();
		SetImports();
		SetPropertyDefinitions();
		SetDatatypeDefinitions();
		SetClassDefinitions();
	}

	// Set declaration
	void SetDeclaration()
	{
		Declaration = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	}

	// Set entity definitions
	void SetEntityDefinitions()
	{
		// DOCTYPE root name
		EntityDefinitions.Name = SLOwl::FPrefixName("rdf", "RDF");
		// Declarations
		EntityDefinitions.EntityPairs.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
		EntityDefinitions.EntityPairs.Add("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
		EntityDefinitions.EntityPairs.Add("owl", "http://www.w3.org/2002/07/owl#");
		EntityDefinitions.EntityPairs.Add("xsd", "http://www.w3.org/2001/XMLSchema#");
		EntityDefinitions.EntityPairs.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	}

	// Set default owl namespace declarations
	void SetNamespaces()
	{
		Namespaces.Add(SLOwl::FAttribute(SLOwl::FPrefixName("xmlns", "computable"),
			SLOwl::FAttributeValue("http://knowrob.org/kb/computable.owl#")));
		Namespaces.Add(SLOwl::FAttribute(SLOwl::FPrefixName("xmlns", "swrl"),
			SLOwl::FAttributeValue("http://www.w3.org/2003/11/swrl#")));
		Namespaces.Add(SLOwl::FAttribute(SLOwl::FPrefixName("xmlns", "rdf"),
			SLOwl::FAttributeValue("http://www.w3.org/1999/02/22-rdf-syntax-ns#")));
		Namespaces.Add(SLOwl::FAttribute(SLOwl::FPrefixName("xmlns", "owl"),
			SLOwl::FAttributeValue("http://www.w3.org/2002/07/owl#")));
		// TODO contd.
	}

	// Set imports
	void SetImports()
	{
		// Prefix name constants
		const SLOwl::FPrefixName OwlImports("owl", "imports");
		const SLOwl::FPrefixName RdfAbout("rdf", "about");
		const SLOwl::FPrefixName RdfResource("rdf", "resource");

		// Create ontology import node
		OntologyImports.Name = SLOwl::FPrefixName("owl", "Ontology");
		OntologyImports.Attributes.Add(SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("http://knowrob.org/kb/u_map.owl")));
		OntologyImports.Comment = TEXT("Ontologies");

		// Add child import nodes
		OntologyImports.ChildNodes.Add(SLOwl::FNode(OwlImports,
			SLOwl::FAttribute(RdfResource, SLOwl::FAttributeValue("package://knowrob_common/owl/knowrob.owl"))));
		OntologyImports.ChildNodes.Add(SLOwl::FNode(OwlImports,
			SLOwl::FAttribute(RdfResource, SLOwl::FAttributeValue("package://knowrob_common/owl/knowrob_u.owl"))));

	};

	// Set property definitions
	void SetPropertyDefinitions()
	{
		// Prefix name constants
		const SLOwl::FPrefixName RdfAbout("rdf", "about");
		const SLOwl::FPrefixName OwlOP("owl", "ObjectProperty");

		// Add comment to first node
		SLOwl::FNode NodeWithComment = SLOwl::FNode(OwlOP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "describedInMap")));
		NodeWithComment.Comment = TEXT("Property Definitions");
		PropertyDefinitions.Add(NodeWithComment);

		PropertyDefinitions.Add(SLOwl::FNode(OwlOP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "describedInMap2"))));
	}

	// Set datatype definitions
	void  SetDatatypeDefinitions()
	{
		// Prefix name constants
		const SLOwl::FPrefixName RdfAbout("rdf", "about");
		const SLOwl::FPrefixName OwlDP("owl", "DatatypeProperty");

		// Add comment to first node
		SLOwl::FNode NodeWithComment = SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "depthOfObject")));
		NodeWithComment.Comment = TEXT("Datatype Definitions");
		DatatypeDefinitions.Add(NodeWithComment);

		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "heightOfObject"))));
		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "heightOfObject"))));
		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "widthOfObject"))));
		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "vectorX"))));
		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "vectorY"))));
		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "vectorZ"))));
		DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "pathToCadModel"))));

	}

	// Set class definitions
	void SetClassDefinitions()
	{
		// Prefix name constants
		const SLOwl::FPrefixName RdfAbout("rdf", "about");
		const SLOwl::FPrefixName OwlClass("owl", "Class");

		// Add comment to first node
		SLOwl::FNode NodeWithComment = SLOwl::FNode(OwlClass, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "SemanticEnvironmentMap")));
		NodeWithComment.Comment = TEXT("Class Definitions");
		ClassDefinitions.Add(NodeWithComment);

		ClassDefinitions.Add(SLOwl::FNode(OwlClass, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "SLMapPerception"))));
		ClassDefinitions.Add(SLOwl::FNode(OwlClass, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "TimePoint"))));
		ClassDefinitions.Add(SLOwl::FNode(OwlClass, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "Vector"))));
	}
};
