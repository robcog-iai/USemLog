// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"

/**
* Dummy event class
*/

class FSLGraspEvent : public ISLEvent
{
public:
	// Default constructor
	FSLGraspEvent();

	// Constructor with initialization
	FSLGraspEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		const uint32 InHandId, const FString& InHandSemId, const FString& InHandClass,
		const uint32 InOtherId, const FString& InOtherSemId, const FString& InOtherClass);

	// Constructor initialization without end time
	FSLGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const uint32 InHandId, const FString& InHandSemId, const FString& InHandClass,
		const uint32 InOtherId, const FString& InOtherSemId, const FString& InOtherClass);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Unique id of the hand
	uint32 HandId;

	// Semantic id of the hand
	FString HandSemId;

	// Semantic Class of the hand
	FString HandClass;

	// Unique id of the grasped item
	uint32 OtherId;

	// Semantic id of the grasped item
	FString OtherSemId;

	// Semantic Class of the grasped item
	FString OtherClass;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FOwlDoc* OutDoc) override;

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;
	/* End IEvent interface */
};