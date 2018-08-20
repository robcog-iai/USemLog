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

	// Event owl prefix (e.g. log from rdf:about="&log;abc123">")
	FString ExperimentPrefix;

	// Used for ontologies (e.g UE-Experiment, from "http://knowrob.org/kb/UE-Experiment.owl#")
	FString ExperimentOntologyName;

	// Experiment unique Id
	FString ExperimentId;

public:
	// Default constructor
	FOwlEvents() {}

	// Init constructor
	FOwlEvents(const FString& InExperimentPrefix,
		const FString& InExperimentOntologyName,
		const FString& InExperimentId) :
		ExperimentPrefix(InExperimentPrefix),
		ExperimentOntologyName(InExperimentOntologyName),
		ExperimentId(InExperimentId)
	{
		SetOntologyNode(InExperimentOntologyName);
	}

	// Destructor
	~FOwlEvents() {}

	// Create experiment node individual
	void AddExperimentIndividual(const FString& InExperimentPrefix, const FString& InExperimentId)
	{
		const FOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlAttributeValue SemMapInd(InExperimentPrefix, InExperimentId);

		// Create semantic map individual
		ExperimentIndividual.Name = OwlNI;
		ExperimentIndividual.AddAttribute(FOwlAttribute(RdfAbout, SemMapInd));
		ExperimentIndividual.AddChildNode(FOwlNode(RdfType, FOwlAttribute(
			RdfResource, FOwlAttributeValue("knowrob", "UnrealExperiment"))));
		ExperimentIndividual.Comment = "Experiment Individual " + InExperimentId;

		// Add map to the document individuals
		Individuals.Add(ExperimentIndividual);
	}
};
