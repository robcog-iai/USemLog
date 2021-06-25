// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SLOwlDoc.h"

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
	//FSLOwlSemanticMap() {}

	// Init constructor
	FSLOwlSemanticMap(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FSLOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FSLOwlSemanticMap() {}
	
	// Create semantic map node individual
	void AddSemanticMapIndividual(const FString& InDescription, const FString& InLevelName)
	{
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName RdfType("rdf", "type");
		const FSLOwlPrefixName RdfResource("rdf", "resource");
		const FSLOwlPrefixName KbMapDescription("knowrob", "mapDescription");
		const FSLOwlPrefixName KbLevelName("knowrob", "levelName");
		const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
		const FSLOwlAttributeValue AttrValString("xsd", "string");
		const FSLOwlAttributeValue SemMapId(Prefix, Id);

		// Create semantic map individual
		SemMapIndividual.Name = OwlNI;
		SemMapIndividual.AddAttribute(FSLOwlAttribute(RdfAbout, SemMapId));
		SemMapIndividual.AddChildNode(FSLOwlNode(RdfType, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue("knowrob", "SemanticEnvironmentMap"))));
		SemMapIndividual.AddChildNode(FSLOwlNode(KbMapDescription,
			FSLOwlAttribute(RdfDatatype, AttrValString), InDescription));
		SemMapIndividual.AddChildNode(FSLOwlNode(KbLevelName,
			FSLOwlAttribute(RdfDatatype, AttrValString), InLevelName));

		SemMapIndividual.Comment = "Semantic Map " + Id;

		// Add map to the document individuals
		Individuals.Add(SemMapIndividual);
	}
};
