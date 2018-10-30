// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"

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
		const uint32 InSupportedObjId, const FString& InSupportedObjSemId, const FString& InSupportedObjClass,
		const uint32 InSupportingObjId, const FString& InSupportingObjSemId, const FString& InSupportingObjClass);

	// Constructor with initialization without end time 
	FSLSupportedByEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const uint32 InSupportedObjId, const FString& InSupportedObjSemId, const FString& InSupportedObjClass,
		const uint32 InSupportingObjId, const FString& InSupportingObjSemId, const FString& InSupportingObjClass);

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Unique id of the supported object
	uint32 SupportedObjId;

	// Semantic id of the supported object
	FString SupportedObjSemId;

	// Semantic class of the supported object
	FString SupportedObjClass;

	// Unique id of the supported object
	uint32 SupportingObjId;

	// Semantic id of the supporting object
	FString SupportingObjSemId;

	// Semantic Class of the object supporting
	FString SupportingObjClass;

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