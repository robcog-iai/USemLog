// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/SphereComponent.h"
#include "SLStructs.h"
#include "SLReachListener.generated.h"


//typedef TPair<float, float> FTimeAndDistance;
//using FTimeAndDistance = TPair<float, float>;
using FTimeAndDistance = TTuple<float, float>; // <Time, Distance>

// Forward declarations
class AStaticMeshActor;

/** Notify when a reaching event happened*/
DECLARE_MULTICAST_DELEGATE_FourParams(FSLReachEventSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*StartTime*/, float /*EndTime*/);

/** Notify when a manipulator positioning event happened*/
DECLARE_MULTICAST_DELEGATE_FourParams(FSLManipulatorPositioningEventSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*StartTime*/, float /*EndTime*/);

/**
 * Checks for reaching actions
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics), DisplayName = "SL Reach Listener")
class USEMLOG_API USLReachListener : public USphereComponent
{
	GENERATED_BODY()

public:
	// Set default values
	USLReachListener();

	// Dtor
	~USLReachListener();

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
	void Update();

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Check if the object is can be a candidate for reaching
	bool CanBeACandidate(AStaticMeshActor* InObject) const;

	// End reach and positioning events, pause timer
	void OnSLGraspBegin(const FSLEntity& Self, UObject* Other, float Time, const FString& GraspType);

	// Reset looking for the events
	void OnSLGraspEnd(const FSLEntity& Self, UObject* Other, float Time);
	
	// Used for the reaching and hand positioning detection
	void OnSLManipulatorContactBegin(const FSLContactResult& ContactResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:
	// Event called when the reaching motion is finished
	FSLReachEventSignature OnReachEvent;

	// Event called when the manipulator positioning motion is finished
	FSLReachEventSignature OnManipulatorPositioningEvent;
	
private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// How often to check for reaching action in its area (0 = every tick)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate;

	// Weight (kg) limit for candidates that can be potentially reached
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float WeightLimit;

	// Volume (cm^3) limit for candidates that can be potentially reach (1000cm^3 = 1 Liter)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float VolumeLimit;

	// Timer handle for the update rate
	FTimerHandle TimerHandle;

	// Semantic data of the owner
	FSLEntity SemanticOwner;

	// CandidatesWithTimeAndDistance for reaching action, pointing to their starting time
	TMap<AStaticMeshActor*, FTimeAndDistance> CandidatesWithTimeAndDistance;

	// Pause everything if the hand is currently grasping something
	UObject* CurrGraspedObj;
	
	/* Constants */
	// Minimal duration for the reaching events
	constexpr static float ReachEventMin = 0.35f;
	// Minimal duration for the positioning events
	constexpr static float ManipulatorPositionEventMin = 0.25f;
	// Minimum distance squared for reaching movements
	constexpr static float MinDistSq = 2.5f * 2.5f;
};
