// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Private/SLUtils.h"
#include "Private/SLOwlUtils.h"

/**
 * Semantic events exporter
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
	void WriteEvents(const FString Path, const float Timestamp, bool bWriteTimelines = false);

	// Add object individual
	void AddObjectIndividual(
		const FString IndividualNs,
		const FString IndividualName,
		const TArray<FSLOwlTriple>& Properties = TArray<FSLOwlTriple>());

	// Add finished event
	void AddFinishedEventIndividual(
		const FString& EventNs,
		const FString& EventName,
		const float StartTime,
		const float EndTime,
		const TArray<FSLOwlTriple>& Properties = TArray<FSLOwlTriple>());
	
	// TODO 
	// BeginEventIndividual
	// EndEventIndividual

	// Enable listening to events
	void SetListenToEvents(bool bListen) {bListenToEvents = bListen;}
		
private:
	// Add finish time to all events
	void TerminateEvents(const float Timestamp);

	// Write events as timelines
	void WriteTimelines(const FString FilePath);

	// Init the metadata object individual of the episode
	void InitMetadata(const float Timestamp);

	// Finish metadata object individual
	void FinishMetadata(const float Timestamp);

	// Add document delcarations
	void AddDocumentDeclarations(rapidxml::xml_document<>* Doc);

	// Add RDF Node attributes
	void AddRDFNodeAttributes(rapidxml::xml_document<>* Doc, rapidxml::xml_node<>* RDFNode);
	
	// Get the timepoint with namespace
	FORCEINLINE const FString GetAsKnowrobTs(const float Timestamp);

	// Create time object
	FSLOwlObjectIndividual* CreateTimeIndividual(const FString TimeObject);

	// Check for designators in the properties
	void CheckForCRAMDesignators(const TArray<FSLOwlTriple>& Properties);

	// Episode unique tag
	FString EpisodeUniqueTag;

	// Reference map of actors to their unique name
	TMap<AActor*, FString> ActToUniqueName;

	// Reference map of actors to their class type
	TMap<AActor*, FString> ActToClassType;

	// Enable listening to events
	bool bListenToEvents;

	// Metadata individual semantic event
	FSLOwlObjectIndividual* MetadataF;

	// Event name to event individuals map
	TMap<FString, FSLOwlEventIndividual*> NameToOpenedEventsMap;

	// Array of all the finished events
	TArray<FSLOwlEventIndividual*> FinishedEvents;

	// Unique objects individuals map
	TMap<FString, FSLOwlObjectIndividual*> ObjIndividualsMap;

};
