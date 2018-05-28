// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlSemanticMapIAI.h"

/**
*
*/
struct FOwlSemanticMapIAIKitchen : public FOwlSemanticMapIAI
{
public:
	// Default constructor
	FOwlSemanticMapIAIKitchen() 
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
		// Add base class default definitions
		FOwlSemanticMapIAI::AddEntityDefinitions();		
		EntityDefinitions.EntityPairs.Add("knowrob", "http://knowrob.org/kb/knowrob-IAIKITCHEN_TODO_TEST.owl#");
	}

	// Add default owl namespace declarations
	void AddNamespaces()
	{
		// Add base class default namespaces
		FOwlSemanticMapIAI::AddNamespaces();	
		FOwlAttributeValue("http://www.w3.org/2002/07/owl-IAIKITCHEN_TODO_TEST.owl#");
		// TODO contd.
	}


	// Add imports
	void AddImports()
	{
		// Add base class default imports
		FOwlSemanticMapIAI::AddImports();

		// Prefix name constants
		const FOwlPrefixName OwlImports("owl", "imports");
		const FOwlPrefixName RdfResource("rdf", "resource");
			
		OntologyImports.ChildNodes.Add(FOwlNode(OwlImports,
			FOwlAttribute(RdfResource, FOwlAttributeValue("package://knowrob_common/owl/knowrob_u_IAI_TODO_KITCHEN.owl"))));
	};

	// Add property definitions
	void AddPropertyDefinitions()
	{
		// Add base class default property definitions
		FOwlSemanticMapIAI::AddPropertyDefinitions();	

		// Prefix name constants
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlOP("owl", "ObjectProperty");

		PropertyDefinitions.Add(FOwlNode(OwlOP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "describedInMapTODO_KITCHEN"))));
	}

	// Add datatype definitions
	void  AddDatatypeDefinitions()
	{
		// Add base class default datatype definitions
		FOwlSemanticMapIAI::AddDatatypeDefinitions();

		// Prefix name constants
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlDP("owl", "DatatypeProperty");
			
		DatatypeDefinitions.Add(FOwlNode(OwlDP, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "pathToCadModel_KITCHEN_TODO"))));
	}

	// Add class definitions
	void AddClassDefinitions()
	{
		// Add base class default class definitions
		FOwlSemanticMapIAI::AddClassDefinitions();

		// Prefix name constants
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName OwlClass("owl", "Class");

		ClassDefinitions.Add(FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", "VectorKITCHNE_TODO_IAI"))));
	}
};
