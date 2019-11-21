// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "SLManipulatorOverlapSphere.generated.h"

/**
* Hand type
*/
UENUM()
enum class ESLManipulatorOverlapGroup : uint8
{
	A					UMETA(DisplayName = "A"),
	B					UMETA(DisplayName = "B"),
};

/**
 * Overlap event end data
 */
struct FSLManipulatorOverlapEndEvent
{
	// Default ctor
	FSLManipulatorOverlapEndEvent() = default;

	// Init ctor
	FSLManipulatorOverlapEndEvent(AActor* InOtherActor, float InTime) :
		OtherActor(InOtherActor), Time(InTime) {};

	// Overlap component
	AActor* OtherActor;

	// End time of the event 
	float Time;
};

/** Delegate to notify that a contact begins between the grasp overlap and an item**/
DECLARE_MULTICAST_DELEGATE_OneParam(FSLManipulatorOverlapBeginSignature, AActor* /*OtherActor*/);

/** Delegate to notify that a contact ended between the grasp overlap and an item**/
DECLARE_MULTICAST_DELEGATE_OneParam(FSLManipulatorOverlapEndSignature, AActor* /*OtherActor*/);

/**
 * Semantic overlap generator for grasp detection
 */
UCLASS(ClassGroup = (SL), meta = (BlueprintSpawnableComponent), DisplayName = "SL Manipulator Overlap Sphere")
class USEMLOG_API USLManipulatorOverlapSphere : public USphereComponent
{
	GENERATED_BODY()

public:
	// Ctor
	USLManipulatorOverlapSphere();

	// Dtor
	~USLManipulatorOverlapSphere() = default;
	
	// Attach to bone 
	bool Init(bool bGrasp = true, bool bContact = true);

	// Start listening to overlaps
	void Start();

	// Pause/continue the overlap detection
	void PauseGrasp(bool bInPause);

	// Stop publishing overlap events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// True if grasp overlaps are paused
	bool IsGraspPaused() const { return bGraspPaused; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Give access to the group of which the shape belongs to for the grasping detection
	ESLManipulatorOverlapGroup GetGroup() const { return Group;};

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	// Attach component to bone
	bool AttachToBone();

	// Set debug color
	void SetColor(FColor Color);

	/* Grasp related*/
	// Publish currently grasp related overlapping components
	void TriggerInitialGraspOverlaps();
	
	// Event called when something starts to overlaps this component
	UFUNCTION()
	void OnGraspOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	void OnGraspOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayedGraspOverlapEndEventCallback();

	// Check if this begin event happened right after the previous one ended
	// if so remove it from the array, and cancel publishing the begin event
	bool SkipRecentGraspOverlapEndEventBroadcast(AActor* OtherActor, float StartTime);

	/* Contact related */
	// Publish currently contact related overlapping components
	void TriggerInitialContactOverlaps();

	// Event called when something starts to overlaps this component
	UFUNCTION()
	void OnContactOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	void OnContactOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayedContactOverlapEndEventCallback();

	// Check if this begin event happened right after the previous one ended
	// if so remove it from the array, and cancel publishing the begin event
	bool SkipRecentContactOverlapEndEventBroadcast(AActor* OtherActor, float StartTime);

public:
	// Grasp related overlap begin/end
	FSLManipulatorOverlapBeginSignature OnBeginManipulatorGraspOverlap;
	FSLManipulatorOverlapEndSignature OnEndManipulatorGraspOverlap;
	
	// Contact related overlap begin/end
	FSLManipulatorOverlapBeginSignature OnBeginManipulatorContactOverlap;
	FSLManipulatorOverlapEndSignature OnEndManipulatorContactOverlap;

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if grasp overlaps are paused
	bool bGraspPaused;

	// True if finished
	bool bIsFinished;

	// Detect grasp contacts (separated since the grasp detection can be paused)
	bool bDetectGrasps;

	// Detect contacts
	bool bDetectContacts;

	// Cache valid contacts
	TSet<AActor*> ActiveContacts;

	// The group of which the shape belongs to for the grasping detection
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLManipulatorOverlapGroup Group;

	// Name of the skeletal bone to attach the shape to
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName BoneName;

	// Snap to the position of the bone when attached
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bSnapToBone;

#if WITH_EDITOR
	// Mimic a button to add the skeletal bone
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bAttachButton;
#endif // WITH_EDITOR

	// If the owner is not a skeletal actor, one needs to add the children (fingers) manually
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bIsNotSkeletal;

	// Explicit references to the other fingers (ignore them during overlap)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bIsNotSkeletal"))
	TArray<AStaticMeshActor*> IgnoreList;


	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle GraspDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLManipulatorOverlapEndEvent> RecentlyEndedGraspOverlapEvents;
	
	
	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle ContactDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLManipulatorOverlapEndEvent> RecentlyEndedContactOverlapEvents;
	

	/* Constants */
	constexpr static bool bVisualDebug = true;
	constexpr static float MaxOverlapEventTimeGap = 0.11f;
};
