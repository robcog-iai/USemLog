// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlDoc.h"

/**
* Semantic events (experiment) document in owl
*/
struct FOwlExperiment : FOwlDoc
{
protected:
	// Array of timepoint individuals
	TArray<FOwlNode> TimepointIndividuals;

	// Set of registered timepoints (in order to avoid multiple individual declaration)
	TSet<float> RegisteredTimepoints;

	// Array of object individuals
	TArray<FOwlNode> ObjectIndividuals;

	// Set of registered objects (in order to avoid multiple individual declaration)
	TSet<uint32> RegisteredObjects;

	// Experiment individual
	FOwlNode ExperimentIndividual;

public:
	// Default constructor
	FOwlExperiment() {}

	// Init constructor
	FOwlExperiment(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FOwlExperiment() {}

	// Add timepoint individual
	bool AddTimepointIndividual(const float Timepoint, const FOwlNode& InOwlNode)
	{
		// Avoid logging the same individual multiple times
		if (!RegisteredTimepoints.Contains(Timepoint))
		{
			RegisteredTimepoints.Add(Timepoint);
			TimepointIndividuals.Emplace(InOwlNode);
			return true;
		}
		return false;
	}

	// Add object individual
	bool AddObjectIndividual(const uint32 ObjUniqueId, const FOwlNode& InOwlNode)
	{
		// Avoid logging the same individual multiple times
		if (!RegisteredObjects.Contains(ObjUniqueId))
		{
			RegisteredObjects.Add(ObjUniqueId);
			ObjectIndividuals.Emplace(InOwlNode);
			return true;
		}
		return false;
	}

	// Create and add experiment node individual
	void AddExperimentIndividual()
	{
		const FOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlAttributeValue ExperimentInd(Prefix, Id);

		// Create experiment individual
		ExperimentIndividual.Name = OwlNI;
		ExperimentIndividual.AddAttribute(FOwlAttribute(RdfAbout, ExperimentInd));
		ExperimentIndividual.AddChildNode(FOwlNode(RdfType, FOwlAttribute(
			RdfResource, FOwlAttributeValue("knowrob", "UnrealExperiment"))));
		ExperimentIndividual.Comment = "Experiment Individual " + Id;

		// Add the experiment to the document individuals
		AddIndividual(ExperimentIndividual);
	}

	// Add time / object individuals to document
	void AddTimepointIndividuals()
	{
		if (TimepointIndividuals.Num() > 0)
		{
			TimepointIndividuals[0].Comment = "Timepoint Individuals";
			AddIndividuals(TimepointIndividuals);
		}

	}

	// Add object individuals
	void AddObjectIndividuals()
	{
		if (ObjectIndividuals.Num() > 0)
		{
			ObjectIndividuals[0].Comment = "Object Individuals";
			AddIndividuals(ObjectIndividuals);
		}
	}
};
