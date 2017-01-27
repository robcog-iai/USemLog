// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Private/SLUtils.h"
#include "Private/SLOwlEntities.h"

// Individual struct
struct IndividualStruct
{
	IndividualStruct(const FString Namespace, const FString UName, 
		const TArray<FSLUtils::SLOwlTriple>& Properties = TArray<FSLUtils::SLOwlTriple>())
		: Ns(Namespace), UniqueName(UName), Properties(Properties)
	{}
	const FString Ns;
	const FString UniqueName;
	TArray<FSLUtils::SLOwlTriple> Properties;
};

// Event struct
struct EventStruct
{
	EventStruct(const FString Namespace, const FString UName, float EvStart = -1.0f, float EvEnd = -1.0f,
		const TArray<FSLUtils::SLOwlTriple>& Properties = TArray<FSLUtils::SLOwlTriple>())
		: Ns(Namespace), UniqueName(UName), Start(EvStart), End(EvEnd), Properties(Properties)
	{}
	const FString Ns;
	const FString UniqueName;
	float Start;
	float End;
	TArray<FSLUtils::SLOwlTriple> Properties;
};

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

	// Write events to file
	void WriteEvents(const FString Path, const float Timestamp, bool bWriteTimelines = true);

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
	
	// Add generic individual
	void AddGenericIndividual(
		const FString IndividualNs,
		const FString IndividualName,
		const TArray<FSLUtils::SLOwlTriple>& Properties = TArray<FSLUtils::SLOwlTriple>());

	// Add generic event with array of properties
	void AddFinishedEvent(
		const FString EventNs,
		const FString EventName,
		const float StartTime,
		const float EndTime,
		const TArray<FSLUtils::SLOwlTriple>& Properties = TArray<FSLUtils::SLOwlTriple>());
	
	// Enable listening to events
	void SetListenToEvents(bool bListen) {bListenToEvents = bListen;}
		
private:
	// Add finish time to all events
	void TerminateEvents(const float Timestamp);

	// Write events as timelines
	void WriteTimelines(const FString FilePath);

	// Add timepoint to array, and return Knowrob specific timestamp
	FORCEINLINE const FString AddTimestamp(const float Timestamp);

	// Get the timepoint with namespace
	FORCEINLINE const FString GetAsKnowrobTs(const float Timestamp);

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


	// Event name to event individuals map
	TMap<FString, FSLOwlEventIndividual*> NameToOpenedEventsMapF;
	// Array of all the finished events
	TArray<FSLOwlEventIndividual*> FinishedEventsF;
	//
	TArray<FSLOwlObjectIndividual*> ObjIndividuals;
	// Metadata individual semantic event
	FSLOwlObjectIndividual* MetadataF;

	void InitMetadata(const float Timestamp);

};
