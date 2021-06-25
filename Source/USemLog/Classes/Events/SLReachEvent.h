// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"

// Forward declarations
class USLBaseIndividual;

/**
* Reach event class
*/
class FSLReachEvent : public ISLEvent
{
public:
	// Default constructor
	FSLReachEvent() = default;

	// Constructor with initialization
	FSLReachEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		USLBaseIndividual* InManipulator, USLBaseIndividual* InIndividual);

	// Constructor initialization without end time
	FSLReachEvent(const FString& InId, const float InStart, const uint64 InPairId,
		USLBaseIndividual* InManipulator, USLBaseIndividual* InIndividual);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Who is Reaching the object
	USLBaseIndividual* Manipulator;

	// The object Reached
	USLBaseIndividual* Individual;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;

	// Send through ROSBridge
	virtual FString ToROSQuery() const override { return ""; };

	// Get event context data as string
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;

	// Get the data as string
	virtual FString ToString() const override;

	// Get the event type name
	virtual FString TypeName() const override { return FString(TEXT("Reach")); };
	/* End IEvent interface */
};