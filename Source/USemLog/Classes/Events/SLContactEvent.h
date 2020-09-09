// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"
#include "SLStructs.h"

/**
* Contact event class
*/
class FSLContactEvent : public ISLEvent
{
public:
	// Default constructor
	FSLContactEvent() = default;

	// Constructor with initialization
	FSLContactEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		const FSLEntity& InItem1, const FSLEntity& InItem2);

	// Constructor initialization without end time
	FSLContactEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const FSLEntity& InItem1, const FSLEntity& InItem2);
	
	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Item1 in contact
	FSLEntity Item1;

	// Item2 in contact
	FSLEntity Item2;

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
	/* End IEvent interface */
};