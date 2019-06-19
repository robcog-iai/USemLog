// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"
#include "SLStructs.h"

/**
* Dummy event class
*/

class FSLSlicingEvent : public ISLEvent
{
public:
	// Default constructor
	FSLSlicingEvent() = default;

	// Constructor with initialization
	FSLSlicingEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
		const FSLEntity& InPerformedBy, const FSLEntity& InDeviceUsed, const FSLEntity& InObjectActedOn,
		const FSLEntity& InOutputsCreated, const bool bInTaskSuccessful);

	// Constructor initialization without endTime, endTaskSuccess and outputsCreated.
	FSLSlicingEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const FSLEntity& InPerformedBy, const FSLEntity& InDeviceUsed, const FSLEntity& InObjectActedOn);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Cutter agent
	FSLEntity PerformedBy;
	
	// Cutter Device
	FSLEntity DeviceUsed;

	// Other item (Sliced item)
	FSLEntity ObjectActedOn;

	// Slice
	FSLEntity OutputsCreated;

	// Task succeeded or not
	bool bTaskSuccessful;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;

	// Get event context data as string (ToString equivalent)
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;

	// Get data as string
	virtual FString ToString() const override;
	/* End IEvent interface */
};