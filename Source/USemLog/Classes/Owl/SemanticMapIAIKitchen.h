// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SemanticMapIAI.h"

namespace SLOwl
{
	/**
	 * 
	 */
	struct FSemanticMapIAIKitchen : public FSemanticMapIAI
	{
	public:
		// Default constructor
		FSemanticMapIAIKitchen() 
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
			FSemanticMapIAI::AddEntityDefinitions();		
			EntityDefinitions.EntityPairs.Add("knowrob", "http://knowrob.org/kb/knowrob-IAIKITCHEN_TODO_TEST.owl#");
		}

		// Add default owl namespace declarations
		void AddNamespaces()
		{
			// Add base class default namespaces
			FSemanticMapIAI::AddNamespaces();	
			SLOwl::FAttributeValue("http://www.w3.org/2002/07/owl-IAIKITCHEN_TODO_TEST.owl#");
			// TODO contd.
		}


		// Add imports
		void AddImports()
		{
			// Add base class default imports
			FSemanticMapIAI::AddImports();

			// Prefix name constants
			const SLOwl::FPrefixName OwlImports("owl", "imports");
			const SLOwl::FPrefixName RdfResource("rdf", "resource");
			
			OntologyImports.ChildNodes.Add(SLOwl::FNode(OwlImports,
				SLOwl::FAttribute(RdfResource, SLOwl::FAttributeValue("package://knowrob_common/owl/knowrob_u_IAI_TODO_KITCHEN.owl"))));
		};

		// Add property definitions
		void AddPropertyDefinitions()
		{
			// Add base class default property definitions
			FSemanticMapIAI::AddPropertyDefinitions();	

			// Prefix name constants
			const SLOwl::FPrefixName RdfAbout("rdf", "about");
			const SLOwl::FPrefixName OwlOP("owl", "ObjectProperty");

			PropertyDefinitions.Add(SLOwl::FNode(OwlOP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "describedInMapTODO_KITCHEN"))));
		}

		// Add datatype definitions
		void  AddDatatypeDefinitions()
		{
			// Add base class default datatype definitions
			FSemanticMapIAI::AddDatatypeDefinitions();

			// Prefix name constants
			const SLOwl::FPrefixName RdfAbout("rdf", "about");
			const SLOwl::FPrefixName OwlDP("owl", "DatatypeProperty");
			
			DatatypeDefinitions.Add(SLOwl::FNode(OwlDP, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "pathToCadModel_KITCHEN_TODO"))));
		}

		// Add class definitions
		void AddClassDefinitions()
		{
			// Add base class default class definitions
			FSemanticMapIAI::AddClassDefinitions();

			// Prefix name constants
			const SLOwl::FPrefixName RdfAbout("rdf", "about");
			const SLOwl::FPrefixName OwlClass("owl", "Class");

			ClassDefinitions.Add(SLOwl::FNode(OwlClass, SLOwl::FAttribute(RdfAbout, SLOwl::FAttributeValue("knowrob", "VectorKITCHNE_TODO_IAI"))));
		}
	};
} // namespace SLOwl