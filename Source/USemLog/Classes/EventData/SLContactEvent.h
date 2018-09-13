// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "ISLEvent.h"

/**
* Contact event class
*/
class FSLContactEvent : public ISLEvent
{
public:
	// Default constructor
	FSLContactEvent();

	// Constructor with initialization
	FSLContactEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId, 
		const uint32 InObj1Id, const FString& InObj1SemId, const FString& InObj1Class,
		const uint32 InObj2Id, const FString& InObj2SemId, const FString& InObj2Class);

	// Constructor initialization without end time
	FSLContactEvent(const FString& InId, const float InStart, const uint64 InPairId,
		const uint32 InObj1Id, const FString& InObj1SemId, const FString& InObj1Class,
		const uint32 InObj2Id, const FString& InObj2SemId, const FString& InObj2Class);
	
	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Unique id of the first object
	uint32 Obj1Id;

	// Semantic id of the first object
	FString Obj1SemId;

	// Semantic Class of the first object
	FString Obj1Class;

	// Unique id of the second object
	uint32 Obj2Id;

	// Semantic id of the second object
	FString Obj2SemId;

	// Semantic Class of the second object
	FString Obj2Class;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FOwlDoc* OutDoc) override;

	// Get event context data as string
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;
	/* End IEvent interface */
};