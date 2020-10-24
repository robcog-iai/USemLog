// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/SphereComponent.h"
#include "SLReachAndPreGraspMonitor.generated.h"

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
	FSLPreGraspEndEvent(USLBaseIndividual* InOther, float InTime) :
		Other(InOther), Time(InTime) {};

	// Other actor
	USLBaseIndividual* Other;

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
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLReachAndPreGraspEventSignature, USLBaseIndividual* /*Self*/, USLBaseIndividual* /*Other*/, float /*ReachStartTime*/, float /*ReachEndTime*/, float /*PreGraspEndTime*/);

/**
 * Checks for reaching actions
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics), DisplayName = "SL Reach And PreGrasp Monitor")
class USEMLOG_API USLReachAndPreGraspMonitor : public USphereComponent
{
	GENERATED_BODY()

public:
	// Set default values
	USLReachAndPreGraspMonitor();

	// Dtor
	~USLReachAndPreGraspMonitor();

protected:
	// Called every frame, used for timeline visualizations, activated and deactivated on request
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
	void Init();

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

	// Set collision parameters such as object name and collision responses
	void SetCollisionParameters();

	// Subscribe for hand contact and grasp events
	bool SubscribeForManipulatorEvents();
	
	// Update callback, checks distance to hand, if it increases it resets the start time
	void UpdateCandidatesData(float DeltaTime);

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Check of the overlapping actor can be a reach candidate
	bool CanBeACandidate(AActor* InOther) const;	
	
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
	void OnManipulatorGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float StartTime, const FString& GraspType);

	// Reset looking for the events
	void OnManipulatorGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float EndTime);
	
	// Used for the reaching and hand positioning detection
	void OnManipulatorContactBegin(const FSLContactResult& ContactResult);

	// Manipulator is not in contact with object anymore, check for possible concatenation, or reset the potential reach time
	void OnManipulatorContactEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float EndTime);
	
	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayContactEndCallback();

	// Check if this begin event happened right after the previous one ended, if so remove it from the array, and cancel publishing the begin event
	bool SkipIfJitterContact(USLBaseIndividual* Other, float StartTime);

public:
	// Event called when the reaching motion is finished
	FSLReachAndPreGraspEventSignature OnReachAndPreGraspEvent;
	
private:
	// Log debug messages
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogDebug : 1;

	// Log verbose debug messages
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogVerboseDebug : 1;

	// Skip initialization if true
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// True if initialized
	uint8 bIsInit : 1;

	// True if started
	uint8 bIsStarted : 1;

	// True if finished
	uint8 bIsFinished : 1;

	// Candidate check update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate;

	// Candidate check update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float ConcatenateIfSmaller;

	// Semantic data component of the owner
	USLIndividualComponent* OwnerIndividualComponent;

	// Semantic individual object
	USLBaseIndividual* OwnerIndividualObject;

	// Individual candidates with information about the possible reaching time and distance form reaching actor
	TMap<USLBaseIndividual*, FSLTimeAndDist> CandidatesData;

	// Individuals and the start timestamp in contact with the manipulator (hand)
	TMap<USLBaseIndividual*, float> ManipulatorContactData;

	// Pause everything if the hand is currently grasping something
	USLBaseIndividual* CurrGraspedIndividual;

	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle DelayTimerHandle;

	// Array of recently ended events
	TArray<FSLPreGraspEndEvent> RecentlyEndedEvents;
	
	/* Constants */
	constexpr static float IgnoreMovementsSmallerThanValue = 2.5f;
	//constexpr static float UpdateRate = 0.037f;
	//constexpr static float ConcatenateIfSmaller = 1.3f;
	static constexpr float ConcatenateIfSmallerDelay = 0.05f;
};
