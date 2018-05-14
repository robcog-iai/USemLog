// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SemanticMap.h"

namespace SLOwl
{
	/**
	 * 
	 */
	struct FSemanticMapIAI : public FSemanticMap
	{
	public:
		// Default constructor
		FSemanticMapIAI() 
		{
			AddDefaultValues();
		}

	protected:
		// Add default values to the map
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
			EntityDefinitions.Name = SLOwl::FPrefixName("rdf", "RDF");
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


		// Add imports
		void AddImports()
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

		// Add property definitions
		void AddPropertyDefinitions()
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

		// Add datatype definitions
		void  AddDatatypeDefinitions()
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

		// Add class definitions
		void AddClassDefinitions()
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
} // namespace SLOwl