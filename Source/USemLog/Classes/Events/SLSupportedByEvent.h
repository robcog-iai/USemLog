// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
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
	FSLSupportedByEvent();

	// Constructor with initialization 
	FSLSupportedByEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		const FSLItem& InSupportedItem, const FSLItem& InSupportingItem);

	// Constructor with initialization without end time 
	FSLSupportedByEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const FSLItem& InSupportedItem, const FSLItem& InSupportingItem);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Supported item
	FSLItem SupportedItem;

	// Supporting item
	FSLItem SupportingItem;

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