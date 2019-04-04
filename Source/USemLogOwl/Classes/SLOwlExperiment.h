// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLOwlDoc.h"

/**
* Events owl document template types
*/
UENUM(BlueprintType)
enum class ESLOwlExperimentTemplate : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	IAI						UMETA(DisplayName = "IAI"),
};

/**
* Semantic events (experiment) document in owl
*/
struct FSLOwlExperiment : FSLOwlDoc
{
protected:
	// Array of timepoint individuals
	TArray<FSLOwlNode> TimepointIndividuals;

	// Set of registered timepoints (in order to avoid multiple individual declaration)
	TSet<float> RegisteredTimepoints;

	// Array of object individuals
	TArray<FSLOwlNode> ObjectIndividuals;

	// Set of registered objects (in order to avoid multiple individual declaration)
	TSet<UObject*> RegisteredObjects;

	// Experiment individual
	FSLOwlNode ExperimentIndividual;

public:
	// Default constructor
	FSLOwlExperiment() {}

	// Init constructor
	FSLOwlExperiment(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FSLOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FSLOwlExperiment() {}

	// Add timepoint individual
	bool AddTimepointIndividual(const float Timepoint, const FSLOwlNode& InOwlNode)
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
	bool AddObjectIndividual(UObject* Object, const FSLOwlNode& InOwlNode)
	{
		// Avoid logging the same individual multiple times
		if (!RegisteredObjects.Contains(Object))
		{
			RegisteredObjects.Add(Object);
			ObjectIndividuals.Emplace(InOwlNode);
			return true;
		}
		return false;
	}

	// Create and add experiment node individual
	void AddExperimentIndividual()
	{
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName RdfType("rdf", "type");
		const FSLOwlPrefixName RdfResource("rdf", "resource");
		const FSLOwlAttributeValue ExperimentInd(Prefix, Id);

		// Create experiment individual
		ExperimentIndividual.Name = OwlNI;
		ExperimentIndividual.AddAttribute(FSLOwlAttribute(RdfAbout, ExperimentInd));
		ExperimentIndividual.AddChildNode(FSLOwlNode(RdfType, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue("knowrob", "UnrealExperiment"))));
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
