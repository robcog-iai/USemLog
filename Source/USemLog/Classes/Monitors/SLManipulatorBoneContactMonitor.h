// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/SphereComponent.h"
#include "SLManipulatorBoneContactMonitor.generated.h"

// Forward declarations
class USLBaseIndividual;
class AStaticMeshActor;
class USkeletalMeshComponent;

/**
* Hand type
*/
UENUM()
enum class ESLManipulatorContactMonitorGroup : uint8
{
	A					UMETA(DisplayName = "A"),
	B					UMETA(DisplayName = "B"),
};

/**
 * Overlap event end data
 */
struct FSLManipulatorContactMonitorEndEvent
{
	// Default ctor
	FSLManipulatorContactMonitorEndEvent() = default;

	// Init ctor
	FSLManipulatorContactMonitorEndEvent(USLBaseIndividual* InOther, float InTimestamp) :
		Other(InOther), Timestamp(InTimestamp) {};

	// Overlap component
	USLBaseIndividual* Other;

	// End time of the event 
	float Timestamp;
};

/** Delegate to notify that a contact begins between the grasp overlap and an item**/
DECLARE_MULTICAST_DELEGATE_OneParam(FSLManipulatorContactMonitorBeginSignature, USLBaseIndividual* /*Other*/);

/** Delegate to notify that a contact ended between the grasp overlap and an item**/
DECLARE_MULTICAST_DELEGATE_OneParam(FSLManipulatorContactMonitorEndSignature, USLBaseIndividual* /*Other*/);

/**
 * Semantic overlap generator for grasp detection
 */
UCLASS(ClassGroup = (SL), meta = (BlueprintSpawnableComponent), DisplayName = "SL Manipulator Bone Contact")
class USEMLOG_API USLManipulatorBoneContactMonitor : public USphereComponent
{
	GENERATED_BODY()

public:
	// Ctor
	USLManipulatorBoneContactMonitor();

	// Dtor
	~USLManipulatorBoneContactMonitor() = default;
	
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
	ESLManipulatorContactMonitorGroup GetGroup() const { return Group;};

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	// Set collision parameters such as object name and collision responses
	void SetCollisionParameters();

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
	bool SkipRecentGraspOverlapEndEventBroadcast(USLBaseIndividual* OtherIndividual, float StartTime);

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
	bool SkipRecentContactOverlapEndEventBroadcast(USLBaseIndividual* OtherIndividual, float StartTime);

public:
	// Grasp related overlap begin/end
	FSLManipulatorContactMonitorBeginSignature OnBeginManipulatorGraspOverlap;
	FSLManipulatorContactMonitorEndSignature OnEndManipulatorGraspOverlap;
	
	// Contact related overlap begin/end
	FSLManipulatorContactMonitorBeginSignature OnBeginManipulatorContactOverlap;
	FSLManipulatorContactMonitorEndSignature OnEndManipulatorContactOverlap;

private:
	// True if initialized
	uint8 bIsInit : 1;

	// True if started
	uint8 bIsStarted : 1;

	// True if grasp overlaps are paused
	uint8 bGraspPaused : 1;

	// True if finished
	uint8 bIsFinished : 1;

	// Detect grasp contacts (separated since the grasp detection can be paused)
	uint8 bDetectGrasps : 1;

	// Detect contacts
	uint8 bDetectContacts : 1;

	// Cache valid contacts
	TSet<USLBaseIndividual*> ActiveContacts;

	// The group of which the shape belongs to for the grasping detection
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLManipulatorContactMonitorGroup Group;

	// Name of the skeletal bone to attach the shape to
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName BoneName;

	// Snap to the position of the bone when attached
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bSnapToBone;

#if WITH_EDITORONLY_DATA
	// Mimic a button to add the skeletal bone
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bAttachButton;
#endif // WITH_EDITORONLY_DATA

	// If the owner is not a skeletal actor, one needs to add the children (fingers) manually
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bIsNotSkeletal;

	// Explicit references to the other fingers (ignore them during overlap)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bIsNotSkeletal"))
	TArray<AStaticMeshActor*> IgnoreList;


	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle GraspDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLManipulatorContactMonitorEndEvent> RecentlyEndedGraspOverlapEvents;
	
	
	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle ContactDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLManipulatorContactMonitorEndEvent> RecentlyEndedContactOverlapEvents;
	

	/* Constants */
	constexpr static bool bVisualDebug = true;
	constexpr static float MaxOverlapEventTimeGap = 0.11f;
};
