// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EventData/ISLEvent.h"

/**
* Contact event class
*/
class FSLContactEvent : public ISLEvent
{
public:
	// Default constructor
	FSLContactEvent();

	// Constructor with initialization
	FSLContactEvent(const FString& InId,
		float InStart,
		float InEnd,
		const FString& InObj1Id,
		const FString& InObj2Id);

	// Semantic id of the first object
	FString Obj1Id;

	// Semantic id of the second object
	FString Obj2Id;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FOwlDoc* OutDoc) override;
	/* End IEvent interface */
};