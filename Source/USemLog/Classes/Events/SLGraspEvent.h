// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"
#include "SLStructs.h"

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
		const FSLItem& InHand, const FSLItem& InOther);

	// Constructor initialization without end time
	FSLGraspEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const FSLItem& InHand, const FSLItem& InOther);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Hand item
	FSLItem Hand;
	
	// Other item (grasped item)
	FSLItem Other;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;
	/* End IEvent interface */
};