// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"
#include "SLStructs.h"

/**
* Supported by event class
*/
class FSLSupportedByEvent : public ISLEvent
{
public:
	// Default constructor
	FSLSupportedByEvent() = default;

	// Constructor with initialization 
	FSLSupportedByEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		const FSLEntity& InSupportedItem, const FSLEntity& InSupportingItem);

	// Constructor with initialization without end time 
	FSLSupportedByEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const FSLEntity& InSupportedItem, const FSLEntity& InSupportingItem);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Supported item
	FSLEntity SupportedItem;

	// Supporting item
	FSLEntity SupportingItem;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;

	// Get the data as string
	virtual FString ToString() const override;
	/* End IEvent interface */
};