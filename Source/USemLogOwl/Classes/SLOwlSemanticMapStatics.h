// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EngineMinimal.h"
#include "SLOwlSemanticMap.h"

/**
* Helper functions for generating semantic maps
*/
struct USEMLOGOWL_API FSLOwlSemanticMapStatics
{
	/* Semantic map template creation */
	// Create Default semantic map
	static TSharedPtr<FSLOwlSemanticMap> CreateDefaultSemanticMap(
		const FString& InDocId,
		const FString& InDocPrefix = "ue-def",
		const FString& InDocOntologyName = "UE-DefaultMap");

	// Create IAI Kitchen semantic map
	static TSharedPtr<FSLOwlSemanticMap> CreateIAIKitchenSemanticMap(
		const FString& InDocId,
		const FString& InDocPrefix = "ue-iai-kitchen",
		const FString& InDocOntologyName = "UE-IAI-Kitchen");

	// Create IAI Supermarket semantic map
	static TSharedPtr<FSLOwlSemanticMap> CreateIAISupermarketSemanticMap(
		const FString& InDocId,
		const FString& InDocPrefix = "ue-iai-supermarket",
		const FString& InDocOntologyName = "UE-IAI-Supermarket");
		
		
	/* Owl individuals / definitions creation */
	// Create an object individual
	static FSLOwlNode CreateObjectIndividual(
		const FString& InDocPrefix, 
		const FString& Id, 
		const FString& Class);

	// Create a pose individual
	static FSLOwlNode CreatePoseIndividual(
		const FString& InDocPrefix, 
		const FString& InId,
		const FVector& InLoc,
		const FQuat& InQuat);

	// Create a constraint individual
	static FSLOwlNode CreateConstraintIndividual(
		const FString& InDocPrefix, 
		const FString& InId,
		const FString& ParentId,
		const FString& ChildId);

	// Create linear constraint properties individual
	static FSLOwlNode CreateLinearConstraintProperties(
		const FString& InDocPrefix, 
		const FString& InId,
		uint8 XMotion,
		uint8 YMotion,
		uint8 ZMotion,
		float Limit,
		bool bSoftConsraint,
		float Stiffness,
		float Damping);

	// Create angular constraint properties individual
	static FSLOwlNode CreateAngularConstraintProperties(
		const FString& InDocPrefix,
		const FString& InId,
		uint8 Swing1Motion,
		uint8 Swing2Motion,
		uint8 TwistMotion,
		float Swing1Limit,
		float Swing2Limit,
		float TwistLimit,
		bool bSoftSwingConstraint,
		float SwingStiffness,
		float SwingDamping,
		bool bSoftTwistConstraint,
		float TwistStiffness,
		float TwistDamping);

	// Create a constraint individual
	static FSLOwlNode CreateClassDefinition(const FString& Class);

	// Create Tags individual
	static FSLOwlNode CreateTagsIndividual(
		const FString& InDocPrefix,
		const FString& Id,
		const FString& Class);

	
	/* Owl properties creation */
	// Create generic property
	static FSLOwlNode CreateGenericProperty(const FSLOwlPrefixName& InPrefixName,
		const FSLOwlAttributeValue& InAttributeValue);
	
	// Create class property
	static FSLOwlNode CreateClassProperty(const FString& InClass);

	// Create describedInMap property
	static FSLOwlNode CreateDescribedInMapProperty(
		const FString& InDocPrefix, const FString& InDocId);

	// Create pathToCadModel property
	static FSLOwlNode CreatePathToCadModelProperty(const FString& InClass);

	// Create tagsData property
	static FSLOwlNode CreateTagsDataProperty(const TArray<FName>& InTags);

	// Create subClassOf property
	static FSLOwlNode CreateSubClassOfProperty(const FString& InSubClassOf);

	// Create skeletal bone property
	static FSLOwlNode CreateSkeletalBoneProperty(const FString& InBone);

	// Create subclass - depth property
	static FSLOwlNode CreateDepthProperty(float Value);

	// Create subclass - height property
	static FSLOwlNode CreateHeightProperty(float Value);

	// Create subclass - width property
	static FSLOwlNode CreateWidthProperty(float Value);

	// Create owl:onProperty meta property
	static FSLOwlNode CreateOnProperty(const FString& InProperty);

	// Create a property with a bool value
	static FSLOwlNode CreateBoolValueProperty(const FSLOwlPrefixName& InPrefixName, bool bValue);

	// Create a property with a int value
	static FSLOwlNode CreateIntValueProperty(const FSLOwlPrefixName& InPrefixName, int32 Value);

	// Create a property with a float value
	static FSLOwlNode CreateFloatValueProperty(const FSLOwlPrefixName& InPrefixName, float Value);

	// Create a property with a string value
	static FSLOwlNode CreateStringValueProperty(const FSLOwlPrefixName& InPrefixName, const FString& InValue);

	// Create pose property
	static FSLOwlNode CreatePoseProperty(const FString& InDocPrefix, const FString& InId);

	// Create linear constraint property
	static FSLOwlNode CreateLinearConstraintProperty(const FString& InDocPrefix, const FString& InId);

	// Create angular constraint property
	static FSLOwlNode CreateAngularConstraintProperty(const FString& InDocPrefix, const FString& InId);

	// Create child property
	static FSLOwlNode CreateChildProperty(const FString& InDocPrefix, const FString& InId);

	// Create parent property
	static FSLOwlNode CreateParentProperty(const FString& InDocPrefix, const FString& InId);

	// Create a location property
	static FSLOwlNode CreateLocationProperty(const FVector& InLoc);

	// Create a quaternion property
	static FSLOwlNode CreateQuaternionProperty(const FQuat& InQuat);
};
