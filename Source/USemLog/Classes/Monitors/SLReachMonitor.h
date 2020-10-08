// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/SphereComponent.h"
#include "SLReachMonitor.generated.h"

// Forward declarations
class AStaticMeshActor;
class USLBaseIndividual;
class USLIndividualComponent;
struct FSLContactResult;


/**
 * PreGrasp event end data
 */
struct FSLPreGraspEndEvent
{
	// Default ctor
	FSLPreGraspEndEvent() = default;

	// Init ctor
	FSLPreGraspEndEvent(AStaticMeshActor* InOther, float InTime) :
		Other(InOther), Time(InTime) {};

	// Other actor
	AStaticMeshActor* Other;

	// End time of the event 
	float Time;
};

// Convenience enum
enum ESLTimeAndDist
{
	SLTime = 0,
	SLDist = 1
};

//typedef TPair<float, float> FSLTimeAndDistance;
//using FSLTimeAndDistance = TPair<float, float>;
using FSLTimeAndDist = TTuple<float, float>; // <Time, Distance>

/** Notify when a reaching event happened*/
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLPreGraspAndReachEventSignature, USLBaseIndividual* /*Self*/, AActor* /*Other*/, float /*ReachStartTime*/, float /*ReachEndTime*/, float /*PreGraspEndTime*/);

/**
 * Checks for reaching actions
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics), DisplayName = "SL Reach Listener")
class USEMLOG_API USLReachMonitor : public USphereComponent
{
	GENERATED_BODY()

public:
	// Set default values
	USLReachMonitor();

	// Dtor
	~USLReachMonitor();

	// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
	bool Init();

	// Start listening to grasp events, update currently overlapping objects
	void Start();

	// Stop publishing grasp events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };
	
protected:
#if WITH_EDITOR
	// UObject interface
	// Called after the C++ constructor and after the properties have been initialized
	virtual void PostInitProperties() override;

	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
#if WITH_EDITOR
	// Move the sphere location so that its surface overlaps with the end of the manipulator
	void RelocateSphere();
#endif // WITH_EDITOR

	// Subscribe for grasp event from sibling component
	bool SubscribeForManipulatorEvents();
	
	// Update callback, checks distance to hand, if it increases it resets the start time
	void ReachUpdate();

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Check if the object is can be a candidate for reaching
	bool CanBeACandidate(AStaticMeshActor* InObject) const;
	
	// Checks for candidates in the overlap area
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Checks for candidates in the overlap area
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	// End reach and positioning events, pause timer
	void OnSLGraspBegin(USLBaseIndividual* Self, AActor* OtherActor, float Time, const FString& GraspType);

	// Reset looking for the events
	void OnSLGraspEnd(USLBaseIndividual* Self, AActor* OtherActor, float Time);
	
	// Used for the reaching and hand positioning detection
	void OnSLManipulatorContactBegin(const FSLContactResult& ContactResult);

	// Manipulator is not in contact with object anymore, check for possible concatenation, or reset the potential reach time
	void OnSLManipulatorContactEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);
	
	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayedManipulatorContactEndEventCallback();

	// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
	bool SkipRecentManipulatorContactEndEventTime(AStaticMeshActor* Other, float StartTime);

public:
	// Event called when the reaching motion is finished
	FSLPreGraspAndReachEventSignature OnPreGraspAndReachEvent;
	
private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Shows if the begin / end overlap callbacks are bound (avoid adding the same callback twice--crash)
	bool bCallbacksAreBound;

	// Semantic data component of the owner
	USLIndividualComponent* IndividualComponent;

	// Semantic individual object
	USLBaseIndividual* IndividualObject;

	// Timer handle for the update rate
	FTimerHandle UpdateTimerHandle;

	// CandidatesWithTimeAndDistance for reaching action, pointing to their starting time
	TMap<AStaticMeshActor*, FSLTimeAndDist> CandidatesWithTimeAndDistance;

	// The objects currently in contact with (before grasping)
	TMap<AStaticMeshActor*, float> ObjectsInContactWithManipulator;
	
	// Pause everything if the hand is currently grasping something
	AActor* CurrGraspedObj;

	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle ManipulatorContactDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLPreGraspEndEvent> RecentlyEndedManipulatorContactEvents;
	
	/* Constants */
	constexpr static float MinDist = 2.5f;
	constexpr static float UpdateRate = 0.27f;
	constexpr static float MaxPreGraspEventTimeGap = 1.3f;
};
