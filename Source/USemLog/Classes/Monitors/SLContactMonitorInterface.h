// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Components/MeshComponent.h"
#include "Components/ShapeComponent.h"
#include "TimerManager.h"
#include "Monitors/SLMonitorStructs.h"
#include "SLContactMonitorInterface.generated.h"

// Forward declaration
class USLBaseIndividual;
class USLIndividualComponent;

// DELEGATES
/** Notiy the begin/end of a supported by event */
DECLARE_MULTICAST_DELEGATE_FourParams(FSLBeginSupportedBySignature, USLBaseIndividual* /*Supported*/, USLBaseIndividual* /*Supporting*/, float /*Time*/, const uint64 /*PairId*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLEndSupportedBySignature, const uint64 /*PairId1*/, const uint64 /*PairId2*/, float /*Time*/);

/**
 * Structure holding the OverlapEnd event data,
 * cached for a small period of time in case it should be concatenated with the follow-up event
 */
struct FSLOverlapEndEvent
{
	// Default ctor
	FSLOverlapEndEvent() = default;

	// Init ctor
	FSLOverlapEndEvent(UPrimitiveComponent* InOtherComp, USLBaseIndividual* InOtherIndividual, float InTime) :
		OtherComp(InOtherComp), OtherIndividual(InOtherIndividual), Time(InTime) {};

	// Overlap component
	UPrimitiveComponent* OtherComp;
	
	// Other item of the overlap end
	USLBaseIndividual* OtherIndividual;

	// Time
	float Time;
};


/**
 *  Unreal style interface for the contact shapes 
 */
UINTERFACE(Blueprintable)
class USLContactMonitorInterface : public UInterface
{
	GENERATED_BODY()
};

class ISLContactMonitorInterface
{
	GENERATED_BODY()

public:
	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	virtual void Init(bool bLogSupportedByEvents = true) = 0;

	// Start publishing overlap events, trigger currently overlapping objects
	virtual void Start() = 0;

	// Stop publishing overlap events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// True if parent is supported by a surface
	bool IsSupportedBySomething() const { return IsSupportedByPariIds.Num() != 0; };

	// Get the last supported by something time
	float GetLastSupportedByEndTime() const { return PrevSupportedByEndTime; }

	// Get the world
	UWorld* GetWorldFromShape() const { return World; };
	
#if WITH_EDITOR
	// Update bounds visual (red/green -- parent is not/is semantically annotated)
	// it is public so it can be accessed from the editor panel for updates
	virtual void UpdateVisualColor() = 0;
#endif // WITH_EDITOR

protected:
#if WITH_EDITOR
	// Load and apply cached parameters from tags
	virtual bool LoadShapeBounds() = 0;

	// Calculate and apply trigger area size
	virtual bool CalcShapeBounds() = 0;

	// Save current parameters to tags
	virtual bool StoreShapeBounds() = 0;
#endif // WITH_EDITOR

	// Init interface
	bool InitContactMonitorInterface(UShapeComponent* InShapeComponent, UWorld* InWorld);

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Start checking for supported by events
	void StartSupportedByUpdateCheck();

	// Check for supported by events
	void SupportedByUpdateCheckBegin();

	// Check if Other is a supported by candidate
	bool CheckAndRemoveIfJustCandidate(USLBaseIndividual* InOther);

	// Event called when something starts to overlaps this component
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayedOverlapEndEventCallback();

	// Broadcast delayed overlaps, if curr time < 0, it guarantees a publish
	bool PublishDelayedOverlapEndEvent(const FSLOverlapEndEvent& Ev, float CurrTime = -1.f);

	// Skip publishing overlap event if it can be concatenated with the current event start
	bool SkipOverlapEndEventBroadcast(USLBaseIndividual* InIndividual, float StartTime);
	
public:
	// Event called when a semantic overlap begins / ends
	FSLBeginContactSignature OnBeginSLContact;
	FSLEndContactSignature OnEndSLContact;

	// Called when a supported by event begins / ends
	FSLBeginSupportedBySignature OnBeginSLSupportedBy;
	FSLEndSupportedBySignature OnEndSLSupportedBy;
	
protected:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Array of events id of objects currently supporting this item, used for checking if this object is supported by any suface(s)
	TArray<uint64> IsSupportedByPariIds;

	// Last supported by time
	float PrevSupportedByEndTime;

	// Pointer to the world
	UWorld* World;

	// Pointer to the given shape component
	UShapeComponent* ShapeComponent;

	// Pointer to the outer (owner) mesh component 
	UMeshComponent* OwnerMeshComp;

	// Semantic data component of the owner
	USLIndividualComponent* IndividualComponent;

	// Semantic individual object
	USLBaseIndividual* IndividualObject;

	// Include supported by events
	bool bLogSupportedByEvents;

	// SupportedBy contact candidates
	TArray<FSLContactResult> SBCandidates;
	
	// Supported by event update timer handle
	FTimerHandle SBTimerHandle;

	// Allow binding against non-UObject functions
	FTimerDelegate SBTimerDelegate;

	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle DelayTimerHandle;

	// Can only bind the timer handle to UObjects or FTimerDelegates
	FTimerDelegate DelayTimerDelegate;

	// Array of recently ended overlaps
	TArray<FSLOverlapEndEvent> RecentlyEndedOverlapEvents;

	/* Constants */
	static constexpr auto TagTypeName = TEXT("SemLogColl");
	static constexpr float SBUpdateRate = 0.11f;
	static constexpr float SBMaxVertSpeed = 0.5f;
	static constexpr float MaxOverlapEventTimeGap = 0.12f;
	static constexpr float MaxOverlapEventTimeGapMult = 1.12f;
};