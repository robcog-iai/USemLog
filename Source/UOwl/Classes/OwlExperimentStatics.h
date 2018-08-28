// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EngineMinimal.h"
#include "OwlExperiment.h"

/**
* Helper functions for generating owl experiment documents
*/
struct UOWL_API FOwlExperimentStatics
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
	static FOwlNode CreateSupportsProperty(
		const FString& InDocPrefix, const FString& InObjId);

	//// Create pathToCadModel property
	//static FOwlNode CreatePathToCadModelProperty(const FString& InClass);

	//// Create subClassOf property
	//static FOwlNode CreateSubClassOfProperty(const FString& InSubClassOf);

	//// Create skeletal bone property
	//static FOwlNode CreateSkeletalBoneProperty(const FString& InBone);

	//// Create subclass - depth property
	//static FOwlNode CreateDepthProperty(float Value);

	//// Create subclass - height property
	//static FOwlNode CreateHeightProperty(float Value);

	//// Create subclass - width property
	//static FOwlNode CreateWidthProperty(float Value);

	//// Create owl:onProperty meta property
	//static FOwlNode CreateOnProperty(const FString& InProperty);

	//// Create a property with a bool value
	//static FOwlNode CreateBoolValueProperty(const FOwlPrefixName& InPrefixName, bool bValue);

	//// Create a property with a int value
	//static FOwlNode CreateIntValueProperty(const FOwlPrefixName& InPrefixName, int32 Value);

	//// Create a property with a float value
	//static FOwlNode CreateFloatValueProperty(const FOwlPrefixName& InPrefixName, float Value);

	//// Create a property with a string value
	//static FOwlNode CreateStringValueProperty(const FOwlPrefixName& InPrefixName, const FString& InValue);

	//// Create pose property
	//static FOwlNode CreatePoseProperty(const FString& InDocPrefix, const FString& InId);

	//// Create linear constraint property
	//static FOwlNode CreateLinearConstraintProperty(const FString& InDocPrefix, const FString& InId);

	//// Create angular constraint property
	//static FOwlNode CreateAngularConstraintProperty(const FString& InDocPrefix, const FString& InId);

	//// Create child property
	//static FOwlNode CreateChildProperty(const FString& InDocPrefix, const FString& InId);

	//// Create parent property
	//static FOwlNode CreateParentProperty(const FString& InDocPrefix, const FString& InId);

	//// Create a location property
	//static FOwlNode CreateLocationProperty(const FVector& InLoc);

	//// Create a quaternion property
	//static FOwlNode CreateQuaternionProperty(const FQuat& InQuat);
};
