// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLOwlDoc.h"

/**
* Semantic map template types
*/
UENUM(BlueprintType)
enum class ESLOwlSemanticMapTemplate : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	IAIKitchen				UMETA(DisplayName = "IAI Kitchen"),
	IAISupermarket			UMETA(DisplayName = "IAI Supermarket"),
};

/**
* Semantic map document in OWL
*/
struct FSLOwlSemanticMap : public FSLOwlDoc
{
protected:
	// SemanticEnvironmentMap individual
	FSLOwlNode SemMapIndividual;

public:
	// Default constructor
	FSLOwlSemanticMap() {}

	// Init constructor
	FSLOwlSemanticMap(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FSLOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FSLOwlSemanticMap() {}
	
	// Create semantic map node individual
	void AddSemanticMapIndividual(const FString& InDocPrefix, const FString& InDocId)
	{
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName RdfType("rdf", "type");
		const FSLOwlPrefixName RdfResource("rdf", "resource");
		const FSLOwlAttributeValue SemMapInd(InDocPrefix, InDocId);

		// Create semantic map individual
		SemMapIndividual.Name = OwlNI;
		SemMapIndividual.AddAttribute(FSLOwlAttribute(RdfAbout, SemMapInd));
		SemMapIndividual.AddChildNode(FSLOwlNode(RdfType, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue("knowrob", "SemanticEnvironmentMap"))));
		SemMapIndividual.Comment = "Semantic Map " + InDocId;

		// Add map to the document individuals
		Individuals.Add(SemMapIndividual);
	}
};
