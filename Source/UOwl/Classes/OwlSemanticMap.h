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
public:
	// SemanticEnvironmentMap individual
	FOwlNode SemMapIndividual;

	// Map prefix (e.g. ue-def from rdf:about="&ue-def;4dfw4smiMD9ne1">)
	FString MapPrefix;

	// Used for ontologies (e.g UE-DefaultMap, from "http://knowrob.org/kb/UE-DefaultMap.owl#")
	FString MapOntologyName;

	// Map unique Id
	FString MapId;

public:
	// Default constructor
	FOwlSemanticMap() {}

	// Init constructor
	FOwlSemanticMap(const FString& InMapPrefix,
		const FString& InMapOntologyName,
		const FString& InMapId) :
		MapPrefix(InMapPrefix),
		MapOntologyName(InMapOntologyName),
		MapId(InMapId)
	{
		SetOntologyNode(InMapOntologyName);
	}

	// Destructor
	~FOwlSemanticMap() {}
	
	// Create semantic map node individual
	void AddSemanticMapIndividual(const FString& InMapPrefix, const FString& InMapId)
	{
		const FOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlAttributeValue SemMapInd(InMapPrefix, InMapId);

		// Create semantic map individual
		SemMapIndividual.Name = OwlNI;
		SemMapIndividual.AddAttribute(FOwlAttribute(RdfAbout, SemMapInd));
		SemMapIndividual.AddChildNode(FOwlNode(RdfType, FOwlAttribute(
			RdfResource, FOwlAttributeValue("knowrob", "SemanticEnvironmentMap"))));
		SemMapIndividual.Comment = "Semantic Map " + InMapId;

		// Add map to the document individuals
		Individuals.Add(SemMapIndividual);
	}
};
