// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Private/SLUtils.h"

/**
 * Semantic map exporter
 */
class FSLEventsExporter
{
public:
	// Constr
	FSLEventsExporter(
		const FString EpisodeUniqueTag,
		const TMap<AActor*, FString>& ActorToUniqueName,
		const TMap<AActor*, TArray<TPair<FString, FString>>>& ActorToSemLogInfo,
		const float Timestamp);

	// Add beginning of touching event
	void BeginTouchingEvent(AActor* TriggerParent, AActor* OtherActor, const float Timestamp);

	// Add end of touching event
	void EndTouchingEvent(AActor* TriggerParent, AActor* OtherActor, const float Timestamp);

	// Add beginning of grasping event
	void BeginGraspingEvent(AActor* Self, AActor* Other, const float Timestamp);

	// Add end of grasping event
	void EndGraspingEvent(AActor* Self, AActor* Other, const float Timestamp);

	// Add furniture state event
	void FurnitureStateEvent(AActor* Furniture, const FString State, const float Timestamp);
	
	// Write events to file
	void WriteEvents(const FString Path, const float Timestamp, bool bWriteTimelines = true);

	// Enable listening to events
	void SetListenToEvents(bool bListen) {bListenToEvents = bListen;}

	// Event struct
	struct EventStruct
	{
		EventStruct(const FString Namespace, const FString UName, float EvStart = -1.0f, float EvEnd = -1.0f)
			: Ns(Namespace), UniqueName(UName), Start(EvStart), End(EvEnd)
		{}
		const FString Ns;
		const FString UniqueName;
		float Start;
		float End;
		TArray<FSLUtils::SLOwlTriple> Properties;
	};
	
private:
	// Add finish time to all events
	void TerminateEvents(const float Timestamp);

	// Write events as timelines
	void WriteTimelines(const FString FilePath);

	// Add timepoint to array, and return Knowrob specific timestamp
	inline const FString AddTimestamp(const float Timestamp);

	// Episode unique tag
	FString EpisodeUniqueTag;

	// Reference map of actors to their unique name
	TMap<AActor*, FString> ActToUniqueName;

	// Reference map of actors to their class type
	TMap<AActor*, FString> ActToClassType;

	// Event name to event individuals map
	TMap<FString, EventStruct*> NameToOpenedEventsMap;

	// Array of all the finished events
	TArray<EventStruct*> FinishedEvents;

	// Array of object individuals
	TArray<AActor*> ObjectIndividuals;

	// Timepoint individuals
	TArray<FString> TimepointIndividuals;

	// Metadata individual semantic event
	EventStruct* Metadata;

	// Enable listening to events
	bool bListenToEvents;
};

