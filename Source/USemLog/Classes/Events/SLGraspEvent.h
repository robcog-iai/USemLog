// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"

// Forward declarations
class USLBaseIndividual;

/**
* Dummy event class
*/

class FSLGraspEvent : public ISLEvent
{
public:
	// Default constructor
	FSLGraspEvent() = default;

	// Constructor with initialization
	FSLGraspEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		USLBaseIndividual* InManipulator, USLBaseIndividual* InIndividual, const FString& InGraspType = "FixationGrasp");

	// Constructor initialization without end time
	FSLGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
		USLBaseIndividual* InManipulator, USLBaseIndividual* InIndividual, const FString& InGraspType = "FixationGrasp");

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Manipulator Individual
	USLBaseIndividual* Manipulator;
	
	// Individual Individual (grasped Individual)
	USLBaseIndividual* Individual;

	// Grasp type
	FString GraspType;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;

	// Send through ROSBridge
	virtual FString ToROSQuery() const override { return ""; };

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;

	// Get data as string
	virtual FString ToString() const override;

	// Get the event type name
	virtual FString TypeName() const override { return FString(TEXT("Grasp")); };
	/* End IEvent interface */
};