// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/SphereComponent.h"
#include "SLBoneContactMonitor.generated.h"

// Forward declarations
class USLBaseIndividual;
class AStaticMeshActor;
class USkeletalMeshComponent;

/**
* Hand type
*/
UENUM()
enum class ESLBoneContactGroup : uint8
{
	A					UMETA(DisplayName = "A"),
	B					UMETA(DisplayName = "B"),
};

/**
 * Overlap event end data
 */
struct FSLBoneContactEndEvent
{
	// Default ctor
	FSLBoneContactEndEvent() = default;

	// Init ctor
	FSLBoneContactEndEvent(USLBaseIndividual* InOther, float InTimestamp) :
		Other(InOther), Timestamp(InTimestamp) {};

	// Overlap component
	USLBaseIndividual* Other;

	// End time of the event 
	float Timestamp;
};

/** Delegate to notify that a contact begins between the grasp overlap and an item**/
DECLARE_MULTICAST_DELEGATE_TwoParams(FSLBoneOverlapBeginSignature, USLBaseIndividual* /*Other*/, const FName& /*BoneName*/);
// OR DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLBoneOverlapBeginSignature, USLBaseIndividual*, Other);

/** Delegate to notify that a contact ended between the grasp overlap and an item**/
DECLARE_MULTICAST_DELEGATE_TwoParams(FSLBoneOverlapEndSignature, USLBaseIndividual* /*Other*/, const FName& /*BoneName*/);
// OR DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLBoneOverlapEndSignature, USLBaseIndividual*, Other);

/**
 * Semantic overlap generator for grasp detection
 */
UCLASS(ClassGroup = (SL), meta = (BlueprintSpawnableComponent), DisplayName = "SL Bone Contact Monitor")
class USEMLOG_API USLBoneContactMonitor : public USphereComponent
{
	GENERATED_BODY()

public:
	// Ctor
	USLBoneContactMonitor();

	// Dtor
	~USLBoneContactMonitor() = default;
	
	// Attach to bone 
	void Init(bool bGrasp = true, bool bContact = true);

	// Start listening to overlaps
	void Start();

	// Pause/continue the overlap detection
	void PauseGraspDetection(bool bNewValue);

	// Stop publishing overlap events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// True if grasp overlaps are paused
	bool IsGraspPaused() const { return bIsGraspDetectionPaused; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Give access to the group of which the shape belongs to for the grasping detection
	ESLBoneContactGroup GetGroup() const { return Group;};

	// Get the bone name attached to
	FName GetAttachedBoneName() const { return BoneName; };

	// Set the bone name attached to
	void SetAttachedBoneNameChecked(const FName& NewName) { BoneName = NewName; };

	// Attach component to bone
	bool AttachToBone();

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	// Set collision parameters such as object name and collision responses
	void SetCollisionParameters();

	// Set debug color
	void SetColor(FColor Color);

	/* Grasp related*/
	// Bind grasp related overlaps
	void BindGraspOverlapCallbacks();

	// Unbind grasp related overlaps
	void UnbindGraspOverlapCallbacks();

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
	bool IsAJitterGrasp(USLBaseIndividual* OtherIndividual, float StartTime);

	/* Contact related */
	// Bind contact related overlaps
	void BindContactOverlapCallbacks();

	// Unbind contact related overlaps
	void UnbindContactOverlapCallbacks();

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
	void DelayContactEndCallback();

	// Check if this begin event happened right after the previous one ended
	// if so remove it from the array, and cancel publishing the begin event
	bool IsAJitterContact(USLBaseIndividual* OtherIndividual, float StartTime);

public:
	// Grasp related overlap begin/end
	FSLBoneOverlapBeginSignature OnBeginGraspBoneOverlap;
	FSLBoneOverlapEndSignature OnEndGraspBoneOverlap;

	// Contact related overlap begin/end
	FSLBoneOverlapBeginSignature OnBeginContactBoneOverlap;
	FSLBoneOverlapEndSignature OnEndContactBoneOverlap;

private:
	// Candidate check update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogContactDebug : 1;

	// Candidate check update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogGraspDebug : 1;

	// True if initialized
	uint8 bIsInit : 1;

	// True if started
	uint8 bIsStarted : 1;

	// True if grasp overlaps are paused
	uint8 bIsGraspDetectionPaused : 1;

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
	ESLBoneContactGroup Group;

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
	TArray<FSLBoneContactEndEvent> RecentlyEndedGraspOverlapEvents;
	
	
	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle ContactDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLBoneContactEndEvent> RecentlyEndedContactOverlapEvents;
	

	/* Constants */
	constexpr static bool bVisualDebug = true;
	constexpr static float ConcatenateIfSmaller = 0.26f;
	constexpr static float ConcatenateIfSmallerDelay = 0.05f;
};
