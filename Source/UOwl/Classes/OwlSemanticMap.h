// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlDoc.h"

/**
* Semantic map document in OWL
*/
struct FOwlSemanticMap : public FOwlDoc
{
protected:
	// SemanticEnvironmentMap individual
	FOwlNode SemMapIndividual;

public:
	// Default constructor
	FOwlSemanticMap() {}

	// Init constructor
	FOwlSemanticMap(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FOwlSemanticMap() {}
	
	// Create semantic map node individual
	void AddSemanticMapIndividual(const FString& InDocPrefix, const FString& InDocId)
	{
		const FOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlAttributeValue SemMapInd(InDocPrefix, InDocId);

		// Create semantic map individual
		SemMapIndividual.Name = OwlNI;
		SemMapIndividual.AddAttribute(FOwlAttribute(RdfAbout, SemMapInd));
		SemMapIndividual.AddChildNode(FOwlNode(RdfType, FOwlAttribute(
			RdfResource, FOwlAttributeValue("knowrob", "SemanticEnvironmentMap"))));
		SemMapIndividual.Comment = "Semantic Map " + InDocId;

		// Add map to the document individuals
		Individuals.Add(SemMapIndividual);
	}
};
