#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EventData/SLSupportedByEvent.h"

/** Delegate for notification of finished semantic contact event */
DECLARE_DELEGATE_OneParam(FSLSupportedByEventSignature, TSharedPtr<FSLSupportedByEvent>);

/**
 * SupportedBy data of the other object
 */
struct FSLSupportedByCandidateData
{
	// Unique id of the object bound to the semantic data
	uint32 Id;

	// Semantic id of the component
	FString SemId;

	// Semantic class of the component
	FString SemClass;

	// Pointer to the static mesh component
	UStaticMeshComponent* StaticMeshComp;

	// Object can only support others
	bool bCanOnlyBeSupporting;
};

/**
 * Semantic contact publisher
 */
class FSLSupportedByPublisher
{
public:
	// Constructor 
	FSLSupportedByPublisher(class USLOverlapArea* InSLOverlapArea);

	// Init
	void Init();

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime);

private:
	// Add supported by candidate (other)
	void AddCandidate(uint32 OtherId,
		const FString& OtherSemId,
		const FString& OtherSemClass,
		bool bCanOnlyBeSupporting);

	// Check if other obj is a supported by candidate
	bool IsACandidate(const uint32 InOtherId, bool bRemoveIfFound = false);

	// Start new contact event
	void StartAndAddEvent(
		const uint32 InOtherId,
		const FString& InOtherSemId,
		const FString& InOtherSemClass,
		float StartTime);

	// Finish then publish the event
	bool FinishAndPublishEvent(const uint32 InOtherId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAndPublishStartedEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLOverlapBegin(const uint32 OtherId,
		const FString& OtherSemId,
		const FString& OtherSemClass,
		float StartTime,
		bool bIsSLOverlapArea);

	// Event called when a semantic overlap event ends
	void OnSLOverlapEnd(const uint32 OtherIdId,
		const FString& SemOtherSemIdId,
		const FString& OtherSemClass,
		float EndTime,
		bool bIsSLOverlapArea);

public:
	// Event called when a supported by event is finished
	FSLSupportedByEventSignature OnSupportedByEvent;

private:
	// Parent semantic overlap area
	class USLOverlapArea* Parent;
	
	// Candidates for supported by event
	TArray<FSLSupportedByCandidateData> Candidates;

	// Array of started supported by events
	TArray<TSharedPtr<FSLSupportedByEvent>> StartedEvents;



	//////////////////////////////////////////////////////////////////////////
	//// Check if a supported by event started from the candidates
	//void SupportedByTimerCb();

	////
	//FTimerDelegate TimerDelegateNextTick;
	//void NextTickCb();
};
