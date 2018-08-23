// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EngineMinimal.h"
#include "OwlEvents.h"

/**
* Helper functions for generating owl events documents
*/
struct UOWL_API FOwlEventsStatics
{
	/* Events doc (experiment) template creation */
	// Create Default experiment
	static TSharedPtr<FOwlEvents> CreateDefaultExperiment(
		const FString& InDocId,
		const FString& InDocPrefix = "log",
		const FString& InDocOntologyName = "Experiment");

	// Create UE experiment
	static TSharedPtr<FOwlEvents> CreateUEExperiment(
		const FString& InDocId,
		const FString& InDocPrefix = "log",
		const FString& InDocOntologyName = "UE-Experiment");
		
		
	/* Owl individuals / definitions creation */
	// Create an event individual
	static FOwlNode CreateEventIndividual(
		const FString& InDocPrefix, 
		const FString& Id, 
		const FString& Class);

	// Create a timepoint individual
	static FOwlNode CreateTimepointIndividual(
		const FString& InDocPrefix,
		const float Timepoint);

	//// Create a constraint individual
	//static FOwlNode CreateConstraintIndividual(
	//	const FString& InDocPrefix, 
	//	const FString& InId,
	//	const FString& ParentId,
	//	const FString& ChildId);

	//// Create linear constraint properties individual
	//static FOwlNode CreateLinearConstraintProperties(
	//	const FString& InDocPrefix, 
	//	const FString& InId,
	//	uint8 XMotion,
	//	uint8 YMotion,
	//	uint8 ZMotion,
	//	float Limit,
	//	bool bSoftConsraint,
	//	float Stiffness,
	//	float Damping);

	//// Create angular constraint properties individual
	//static FOwlNode CreateAngularConstraintProperties(
	//	const FString& InDocPrefix,
	//	const FString& InId,
	//	uint8 Swing1Motion,
	//	uint8 Swing2Motion,
	//	uint8 TwistMotion,
	//	float Swing1Limit,
	//	float Swing2Limit,
	//	float TwistLimit,
	//	bool bSoftSwingConstraint,
	//	float SwingStiffness,
	//	float SwingDamping,
	//	bool bSoftTwistConstraint,
	//	float TwistStiffness,
	//	float TwistDamping);

	//// Create a constraint individual
	//static FOwlNode CreateClassDefinition(const FString& Class);

	
	/* Owl properties creation */
	// Create class property
	static FOwlNode CreateClassProperty(const FString& InClass);

	// Create startTime property
	static FOwlNode CreateStartTimeProperty(
		const FString& InDocPrefix, const float Timepoint);

	// Create endTime property
	static FOwlNode CreateEndTimeProperty(
		const FString& InDocPrefix, const float Timepoint);

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
