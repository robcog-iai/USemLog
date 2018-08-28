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
		const FString& InObj1Class,
		const uint32 InObj1UniqueId,
		const FString& InObj2Id,
		const FString& InObj2Class,
		const uint32 InObj2UniqueId);

	// Semantic id of the first object
	FString Obj1Id;

	// Unique id of the first object
	uint32 Obj1UniqueId;

	// Semantic Class of the first object
	FString Obj1Class;

	// Semantic id of the second object
	FString Obj2Id;

	// Semantic Class of the second object
	FString Obj2Class;

	// Unique id of the second object
	uint32 Obj2UniqueId;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FOwlDoc* OutDoc) override;
	/* End IEvent interface */
};