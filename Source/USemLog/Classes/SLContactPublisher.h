// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EventData/SLContactEvent.h"

/** Delegate for notification of finished semantic contact event */
DECLARE_DELEGATE_OneParam(FSLContactEventSignature, TSharedPtr<FSLContactEvent>);

// Forward declarations
struct FSLOverlapResult;

/**
 * Semantic contact publisher
 */
class FSLContactPublisher 
{
public:
	// Constructor 
	FSLContactPublisher(class USLOverlapArea* InSLOverlapArea);

	// Init
	void Init();

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime);

private:
	// Start new contact event
	//void StartAndAddEvent(
	//	const uint32 InOtherId,
	//	const FString& InOtherSemId,
	//	const FString& InOtherSemClass,
	//	float StartTime);
	void StartAndAddNewEvent(const FSLOverlapResult& SemanticOverlapBeginResult);


	// Finish then publish the event
	bool FinishAndPublishEvent(const uint32 InOtherId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAndPublishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLOverlapBegin(const FSLOverlapResult& SemanticOverlapBeginResult);
	//void OnSLOverlapBegin(UStaticMeshComponent* OtherStaticMeshComp, 
	//	const uint32 OtherId,
	//	const FString& OtherSemId,
	//	const FString& OtherSemClass,
	//	float StartTime,
	//	bool bIsSLOverlapArea);

	// Event called when a semantic overlap event ends
	void OnSLOverlapEnd(const FSLOverlapResult& SemanticOverlapEndResult);
	//void OnSLOverlapEnd(const uint32 OtherIdId,
	//	const FString& SemOtherSemIdId,
	//	const FString& OtherSemClass,
	//	float EndTime,
	//	bool bIsSLOverlapArea);

public:
	// Event called when a semantic contact event is finished
	FSLContactEventSignature OnSemanticContactEvent;

private:
	// Parent semantic overlap area
	class USLOverlapArea* Parent;

	// Array of started contact events
	TArray<TSharedPtr<FSLContactEvent>> StartedEvents;
};
