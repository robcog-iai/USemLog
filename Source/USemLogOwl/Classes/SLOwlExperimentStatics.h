// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EngineMinimal.h"
#include "OwlExperiment.h"

/**
* Helper functions for generating owl experiment documents
*/
struct USEMLOGOWL_API FSLOwlExperimentStatics
{
	/* Events doc (experiment) template creation */
	// Create Default experiment
	static TSharedPtr<FOwlExperiment> CreateDefaultExperiment(
		const FString& InDocId,
		const FString& InDocPrefix = "log",
		const FString& InDocOntologyName = "Experiment");

	// Create UE experiment
	static TSharedPtr<FOwlExperiment> CreateUEExperiment(
		const FString& InDocId,
		const FString& InDocPrefix = "log",
		const FString& InDocOntologyName = "UE-Experiment");
		
		
	/* Owl individuals / definitions creation */
	// Create an event individual
	static FOwlNode CreateEventIndividual(
		const FString& InDocPrefix, 
		const FString& InId,
		const FString& InClass);

	// Create a timepoint individual
	static FOwlNode CreateTimepointIndividual(
		const FString& InDocPrefix,
		const float Timepoint);

	// Create an object individual
	static FOwlNode CreateObjectIndividual(
		const FString& InDocPrefix,
		const FString& InId,
		const FString& InClass);


	/* Owl properties creation */
	// Create class property
	static FOwlNode CreateClassProperty(const FString& InClass);

	// Create startTime property
	static FOwlNode CreateStartTimeProperty(
		const FString& InDocPrefix, const float Timepoint);

	// Create endTime property
	static FOwlNode CreateEndTimeProperty(
		const FString& InDocPrefix, const float Timepoint);

	// Create inContact property
	static FOwlNode CreateInContactProperty(
		const FString& InDocPrefix, const FString& InObjId);

	// Create isSupported property
	static FOwlNode CreateIsSupportedProperty(
		const FString& InDocPrefix, const FString& InObjId);

	// Create supports property
	static FOwlNode CreateIsSupportingProperty(
		const FString& InDocPrefix, const FString& InObjId);
	
	// Create performedBy property
	static FOwlNode CreatePerformedByProperty(
		const FString& InDocPrefix, const FString& InObjId);

	// Create objectActedOn property
	static FOwlNode CreateObjectActedOnProperty(const FString& InDocPrefix, const FString& InObjId);
};
