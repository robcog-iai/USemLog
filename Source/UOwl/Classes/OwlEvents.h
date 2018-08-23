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

	// TODO Start these can be moved in the OwlDoc base class
	// Event owl prefix (e.g. log from rdf:about="&log;abc123">")
	FString DocPrefix;

	// Used for ontologies (e.g UE-Experiment, from "http://knowrob.org/kb/UE-Experiment.owl#")
	FString DocOntologyName;

	// Experiment unique Id
	FString DocId;
	// TODO End these can be moved in the OwlDoc base class

public:
	// Default constructor
	FOwlEvents() {}

	// Init constructor
	FOwlEvents(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		DocPrefix(InDocPrefix),
		DocOntologyName(InDocOntologyName),
		DocId(InDocId)
	{
		SetOntologyNode(InDocOntologyName);
	}

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
