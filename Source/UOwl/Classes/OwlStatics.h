// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EngineMinimal.h"
#include "OwlStructs.h"
#include "OwlNode.h"

/**
* Helper functions for adding owl nodes / properties
*/
struct UOWL_API FOwlStatics
{
	/* Owl individuals creation */
	// Create an object individual
	static FOwlNode CreateObjectIndividual(const FString& Id, const FString& Class);

	// Create a pose individual
	static FOwlNode CreatePoseIndividual(const FString& InId, const FVector& InLoc, const FQuat& InQuat);

	// Create a constraint individual
	static FOwlNode CreateConstraintIndividual();


	/* Owl properties creation */
	// Create class property
	static FOwlNode CreateClassProperty(const FString& InClass);

	// Create pose property
	static FOwlNode CreatePoseProperty(const FString& InId);

	// Create child property
	static FOwlNode CreateChildProperty(const FString& InId);

	// Create parent property
	static FOwlNode CreateParentProperty(const FString& InId);

	// Create a location property
	static FOwlNode CreateLocationProperty(const FVector& InLoc);

	// Create a quaternion property
	static FOwlNode CreateQuaternionProperty(const FQuat& InQuat);
};
