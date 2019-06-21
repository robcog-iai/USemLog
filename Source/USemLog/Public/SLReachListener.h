// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "SLReachListener.generated.h"

// Forward declarations
class AStaticMeshActor;

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

	// Update callback, check for changes in the reach model
	void Update();

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Check if the object is can be a candidate for reaching
	bool CanBeACandidate(AStaticMeshActor* InObject);

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

	// Candidates for reaching action, pointing to their starting time
	TMap<AStaticMeshActor*, float> Candidates;

};
