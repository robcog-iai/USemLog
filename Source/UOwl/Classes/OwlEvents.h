// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlDoc.h"

/**
* Semantic events (experiment) document in owl
*/
struct FOwlEvents : FOwlDoc
{
public:
	// Experiment individual
	FOwlNode ExperimentIndividual;

public:
	// Default constructor
	FOwlEvents() {}

	// Init constructor
	FOwlEvents(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FOwlEvents() {}

	// Create experiment node individual
	void AddExperimentIndividual(const FString& InDocPrefix, const FString& InDocId)
	{
		const FOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlAttributeValue SemMapInd(InDocPrefix, InDocId);

		// Create semantic map individual
		ExperimentIndividual.Name = OwlNI;
		ExperimentIndividual.AddAttribute(FOwlAttribute(RdfAbout, SemMapInd));
		ExperimentIndividual.AddChildNode(FOwlNode(RdfType, FOwlAttribute(
			RdfResource, FOwlAttributeValue("knowrob", "UnrealExperiment"))));
		ExperimentIndividual.Comment = "Experiment Individual " + InDocId;

		// Add map to the document individuals
		Individuals.Add(ExperimentIndividual);
	}
};
