// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SLOwlDoc.h"
#include "Individuals/Type/SLBaseIndividual.h"

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
	TArray<float> RegisteredTimepoints;

	// Array of object individuals
	TArray<FSLOwlNode> ObjectIndividuals;

	// Set of registered objects (in order to avoid multiple individual declaration)
	TSet<USLBaseIndividual*> RegisteredObjects;

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

	// Add timepoint individual value
	void RegisterTimepoint(const float Timepoint)
	{
		RegisteredTimepoints.AddUnique(Timepoint);
	}

	// Add individual instalce value
	bool RegisterObject(USLBaseIndividual* BI)
	{
		// Avoid logging the same individual multiple times
		bool bIsAlreadyInSet = false;
		RegisteredObjects.Add(BI, &bIsAlreadyInSet);
		return bIsAlreadyInSet;
	}

	// Create and add experiment node individual
	void AddExperimentIndividual(const TArray<FString>& SubActionIds, const FString& SemMapId, const FString& TaskId)
	{
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName RdfType("rdf", "type");
		const FSLOwlPrefixName KrPerformedInMap("knowrob", "performedInMap");
		const FSLOwlPrefixName KrPerformedTask("knowrob", "performedTask");
		const FSLOwlPrefixName KrSubAction("knowrob", "subAction");
		const FSLOwlPrefixName KrStartTime("knowrob", "startTime");
		const FSLOwlPrefixName KrEndTime("knowrob", "endTime");
		const FSLOwlPrefixName RdfResource("rdf", "resource");
		const FSLOwlAttributeValue ExperimentId(Prefix, Id);

		// Create experiment individual
		ExperimentIndividual.Name = OwlNI;
		ExperimentIndividual.AddAttribute(FSLOwlAttribute(RdfAbout, ExperimentId));
		ExperimentIndividual.AddChildNode(FSLOwlNode(RdfType, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue("knowrob", "AmevaExperiment"))));

		// Add semantic map id
		ExperimentIndividual.AddChildNode(FSLOwlNode(KrPerformedInMap, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue(Prefix, SemMapId))));

		// Add executed task id
		ExperimentIndividual.AddChildNode(FSLOwlNode(KrPerformedTask, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue(Prefix, TaskId))));

		// Add start and end time
		if (RegisteredTimepoints.Num() > 2)
		{
			float StartTime = RegisteredTimepoints[0];
			const FString StartTimeId = "timepoint_" + FString::SanitizeFloat(StartTime);
			ExperimentIndividual.AddChildNode(FSLOwlNode(KrStartTime,
				FSLOwlAttribute(RdfResource, FSLOwlAttributeValue("log", StartTimeId))));

			float EndTime = RegisteredTimepoints.Last();
			const FString EndTimeId = "timepoint_" + FString::SanitizeFloat(EndTime);
			ExperimentIndividual.AddChildNode(FSLOwlNode(KrEndTime,
				FSLOwlAttribute(RdfResource, FSLOwlAttributeValue("log", EndTimeId))));
		}

		// Add subactions
		for (const auto& SubActionId : SubActionIds)
		{
			ExperimentIndividual.AddChildNode(FSLOwlNode(KrSubAction, FSLOwlAttribute(
				RdfResource, FSLOwlAttributeValue("log", SubActionId))));
		}

		ExperimentIndividual.Comment = "Experiment Individual " + Id;

		// Add the experiment to the document individuals
		AddIndividual(ExperimentIndividual);
	}

	// Add time / object individuals to document
	void AddTimepointIndividuals()
	{
		CreateTimepointIndividuals();
		if (TimepointIndividuals.Num() > 0)
		{
			TimepointIndividuals[0].Comment = "Timepoint Individuals";
			AddIndividuals(TimepointIndividuals);
		}
	}

	// Add object individuals
	void AddObjectIndividuals()
	{
		CreateObjectIndividuals();
		if (ObjectIndividuals.Num() > 0)
		{
			ObjectIndividuals[0].Comment = "Object Individuals";
			AddIndividuals(ObjectIndividuals);
		}
	}

private:
	// Create timepoint individuals from the registered timepoints
	void CreateTimepointIndividuals()
	{
		// Individuals already created
		if (TimepointIndividuals.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Timepoint individuals already created.."), *FString(__FUNCTION__), __LINE__);
			return;
		}

		// Sort timestamps
		//RegisteredTimepoints.Sort([](const float& A, const float& B) { return A < B; });
		RegisteredTimepoints.StableSort();

		// Create and add time individuals
		for (float Ts : RegisteredTimepoints)
		{
			TimepointIndividuals.Add(CreateTimepointIndividual("log", Ts));
		}
	}

	// Create timepoint individuals from the registered timepoints
	void CreateObjectIndividuals()
	{
		// Individuals already created
		if (ObjectIndividuals.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Object individuals already created.."), *FString(__FUNCTION__), __LINE__);
			return;
		}

		// Create and add time individuals
		for (const auto& BI : RegisteredObjects)
		{
			ObjectIndividuals.Add(CreateObjectIndividual("log", BI->GetIdValue(), BI->GetClassValue()));
		}
	}

public:
	/* Static helper functions */
	// Create a timepoint individual
	static FSLOwlNode CreateTimepointIndividual(const FString& InDocPrefix, const float Timepoint)
	{
		// Prefix name constants
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
		FSLOwlNode Individual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, Id)));
		Individual.AddChildNode(FSLOwlNode::CreateResourceProperty("knowrob", "Timepoint"));
		return Individual;
	}

	// Create an object individual
	static FSLOwlNode CreateObjectIndividual(const FString& InDocPrefix, const FString& InId, const FString& InClass)
	{
		// Prefix name constants
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

		FSLOwlNode Individual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, InId)));
		Individual.AddChildNode(FSLOwlNode::CreateResourceProperty("knowrob", InClass));
		return Individual;
	}
};
