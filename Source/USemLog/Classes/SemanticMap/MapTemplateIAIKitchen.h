#pragma once

#include "Owl/Owl.h"
#include "Owl/Doc.h"
#include "SemanticMap/MapTemplateDefault.h"

/**
* Structure containing the default values for a semantic map
*/
struct FMapTemplateIAIKitchen : public FMapTemplateDefault
{
	// Constructor
	FMapTemplateIAIKitchen()
	{
		Build();
	}

	// Output constructor
	FMapTemplateIAIKitchen(FString& OutDeclaration,
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


private:
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
		FMapTemplateDefault::SetDeclaration();
	}

	// Set entity definitions
	void SetEntityDefinitions()
	{
		FMapTemplateDefault::SetEntityDefinitions();
	}

	// Set default owl namespace declarations
	void SetNamespaces()
	{
		FMapTemplateDefault::SetNamespaces();
	}

	// Set imports
	void SetImports()
	{
		FMapTemplateDefault::SetImports();
	};

	// Set property definitions
	void SetPropertyDefinitions()
	{
		FMapTemplateDefault::SetPropertyDefinitions();
	}

	// Set datatype definitions
	void  SetDatatypeDefinitions()
	{
		FMapTemplateDefault::SetDatatypeDefinitions();
	}

	// Set class definitions
	void SetClassDefinitions()
	{
		FMapTemplateDefault::SetClassDefinitions();
	}
};
