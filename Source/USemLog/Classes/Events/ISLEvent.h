// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Owl/SLOwlDoc.h"

/**
* Abstract class ensuring every event can be represented as an Owl Node;
*/
class ISLEvent
{
public:
	// Default constructor
	ISLEvent() {};
	
	// Init constructor
	ISLEvent(const FString& InId, float InStartTime, float InEndTime) 
		: Id(InId), StartTime(InStartTime), EndTime(InEndTime) {};

	// Init without end constructor
	ISLEvent(const FString& InId, float InStartTime)
		: Id(InId), StartTime(InStartTime) {};

	// Virtual destructor
	virtual ~ISLEvent() {};

	// Unique id of the event
	FString Id;

	// Start time of the event
	float StartTime;

	// End time of the event
	float EndTime;

	// Id of the episode
	FString EpisodeId;

	// Create owl representation of the event
	virtual FSLOwlNode ToOwlNode() const = 0;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) = 0;

	// Convert event to ROS Message
	virtual FString ToROSQuery() const = 0;

	// Get event context data (unique name that can repeat of the event type, e.g. Contact_BetweenTheseTwo)
	virtual FString Context() const = 0;

	// Get the tooltip data (extra info that can appear in the google charts)
	virtual FString Tooltip() const = 0;
		
	// To string
	virtual FString ToString() const = 0;

	// Type name
	virtual FString TypeName() const = 0;
};