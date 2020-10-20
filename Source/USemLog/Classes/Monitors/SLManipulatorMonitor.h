// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/ActorComponent.h"
#include "Monitors/SLMonitorStructs.h"
#include "TimerManager.h"
#include "SLManipulatorMonitor.generated.h"

// Forward declarations
class AActor;
class USLBaseIndividual;
class USLIndividualComponent;
class AStaticMeshActor;
class USLManipulatorBoneContactMonitor;
class UPhysicsConstraintComponent; // AdHoc grasp helper
class UStaticMeshComponent; // AdHoc grasp helper
class USkeletalMeshComponent; // AdHoc grasp helper

/**
* Hand type
*/
UENUM()
enum class ESLGraspHandType : uint8
{
	Left					UMETA(DisplayName = "Left"),
	Right					UMETA(DisplayName = "Right"),
};

/**
 * Grasp event end data
 */
struct FSLGraspEndEvent
{
	// Default ctor
	FSLGraspEndEvent() = default;

	// Init ctor
	FSLGraspEndEvent(USLBaseIndividual* InOther, float InTime) :
		Other(InOther), Time(InTime) {};

	// Overlap component
	USLBaseIndividual* Other;

	// End time of the event 
	float Time;
};

/**
 * Contact event end data
 */
struct FSLContactEndEvent
{
	// Default ctor
	FSLContactEndEvent() = default;

	// Init ctor
	FSLContactEndEvent(USLBaseIndividual* InOther, float InTime) :
		Other(InOther), Time(InTime) {};

	// Overlap component
	USLBaseIndividual* Other;

	// End time of the event 
	float Time;
};

/** Notify when an object is grasped and released*/
DECLARE_MULTICAST_DELEGATE_FourParams(FSLBeginManipulatorGraspSignature, USLBaseIndividual* /*Self*/, USLBaseIndividual* /*Other*/, float /*Time*/, const FString& /*Type*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLEndManipulatorGraspSignature, USLBaseIndividual* /*Self*/, USLBaseIndividual* /*Other*/, float /*Time*/);

/**
 * Checks for manipulator related events (contact, grasp)
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Manipulator Monitor")
class USEMLOG_API USLManipulatorMonitor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLManipulatorMonitor();

	// Dtor
	~USLManipulatorMonitor();

	// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
	void Init(bool bInDetectGrasps, bool bInDetectContacts);

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
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Load overlap groups, return true if at least one valid overlap is in each group
	bool LoadOverlapGroups();

	/* Begin grasp related */
#if SL_WITH_MC_GRASP
	// Subscribe to grasp type changes
	bool SubscribeToGraspTypeChanges();

	// Callback on grasp type change
	void OnGraspType(const FString& Type);
#endif // SL_WITH_MC_GRASP

private:
	// Bind user inputs
	void SetupInputBindings();

	// Pause/continue the grasp detection
	void PauseGraspDetection(bool bInPause);

	// Check if the grasp trigger is active
	void GraspInputAxisCallback(float Value);

	// Process beginning of grasp related contact in group A
	UFUNCTION()
	void OnGraspGroupAOverlapBegin(USLBaseIndividual* OtherIndividual);

	// Process beginning of grasp related contact in group B
	UFUNCTION()
	void OnBeginOverlapGroupBGrasp(USLBaseIndividual* OtherIndividual);
	
	// Process ending of grasp related contact in group A
	UFUNCTION()
	void OnGraspGroupAOverlapEnd(USLBaseIndividual* OtherIndividual);

	// Process ending of grasp related  contact in group B
	UFUNCTION()
	void OnEndOverlapGroupBGrasp(USLBaseIndividual* OtherIndividual);
	
	// Grasp update check
	void CheckGraspState();
	
	// A grasp has started
	void GraspStarted(USLBaseIndividual* OtherIndividual);
	
	// A grasp has ended
	void GraspEnded(USLBaseIndividual* OtherIndividual);

	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayedGraspEndCallback();

	// Check if this begin event happened right after the previous one ended
	// if so remove it from the array, and cancel publishing the begin event
	bool SkipIfJitterGrasp(USLBaseIndividual* OtherIndividual, float StartTime);
	/* End grasp related */

	/* Begin contact related */
	// Process beginning of contact
	UFUNCTION()
	void OnContactBoneOverlapBegin(USLBaseIndividual* OtherIndividual);

	// Process ending of contact
	UFUNCTION()
	void OnContactBoneOverlapEnd(USLBaseIndividual* OtherIndividual);

	// Delayed call of sending the finished event to check for possible concatenation of jittering events of the same type
	void DelayedContactEndCallback();

	// Check if this begin event happened right after the previous one ended
	// if so remove it from the array, and cancel publishing the begin event
	bool SkipIfJitterContact(USLBaseIndividual* InOther, float StartTime);
	/* End contact related */

	/* Begin Ad Hoc grasp help */
	// Setup the grasp helper constraint
	bool InitGraspHelper();

	// Grasp help input trigger manual override
	void GraspHelperInputCallback();

	// Start grasp help
	void StartGraspHelper();

	// Stop grasp help
	void StopGraspHelper();
	/* End Ad Hoc grasp help */
	
public:
	// Event called when grasp begins/ends
	FSLBeginManipulatorGraspSignature OnBeginManipulatorGrasp;
	FSLEndManipulatorGraspSignature OnEndManipulatorGrasp;

	// Event called when a semantic overlap/contact begins/ends
	FSLBeginContactSignature OnBeginManipulatorContact;
	FSLEndContactSignature OnEndManipulatorContact;
	
private:
	// Candidate check update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bLogDebug : 1;

	// Skip initialization if true
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// True if initialized
	uint8 bIsInit : 1;

	// True if started
	uint8 bIsStarted : 1;

	// True if finished
	uint8 bIsFinished : 1;

	// True grasp detection is paused
	uint8 bIsPaused : 1;

	// New information added 
	uint8 bGraspIsDirty : 1;

	// Detect grasp contacts
	uint8 bDetectGrasps : 1;

	// Detect contacts
	uint8 bDetectContacts : 1;

	// Ad Hoc grasp helper is active or not
	uint8 bAdHocGraspHelpIsActive : 1;
		
#if WITH_EDITORONLY_DATA
	// Hand type to load pre-defined parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLGraspHandType HandType;
#endif // WITH_EDITORONLY_DATA
	
	// Read the input directly, avoid biding to various controllers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName InputAxisName;

	// Axis input value to wake up from idle
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float InputAxisTriggerThresholdValue;

	// If the owner is not a skeletal actor, one needs to add the children (fingers) manually
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bIsNotSkeletal;

	// Explicit reference to the children (fingers) if the owner is not a skeletal actor
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bIsNotSkeletal"))
	TArray<AStaticMeshActor*> Fingers;

	// Concatenate grasp events with jitters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float GraspConcatenateIfSmaller;

	// Concatenate contact events with jitters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float ContactConcatenateIfSmaller;

	/* Begin Ad Hoc Grasp Helper */
	// Help out with the grasping
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper")
	bool bUseGraspHelper;

	// Use manual override
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper")
	bool bGraspHelperManualOverride;

	// Use manual override for event publishing
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper")
	bool bGraspHelperEventManualOverride;

	// Input action name (uses manual override)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	FName GraspHelperInputActionName;

	// Attach constraint to the given bone
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	FName GraspHelperHandBoneName;

	// Disable gravity on grasp
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	bool bGraspHelperItemDisableGravity;

	// Decrease mass of object
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	bool bGraspHelperItemScaleMass;

	// Decrease mass of grasped object
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	float GraspHelperItemMassScaleValue;

	// Constraint movement limit (all axis)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	float GraspHelperConstraintLimit;

	// AdHoc grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	float GraspHelperConstraintStiffness;

	// AdHoc grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	float GraspHelperConstraintDamping;

	// AdHoc grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	float GraspHelperConstraintContactDistance;

	// AdHoc grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Grasp Helper", meta = (editcondition = "bUseGraspHelper"))
	bool bGraspHelperConstraintParentDominates;

	// AdHoc grasp helper constraint component
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger|Grasp Helper"*/)
	UPhysicsConstraintComponent* GraspHelperConstraint;

	// AdHoc grasped object
	UStaticMeshComponent* GraspHelperItemSMC;

	// Owner skeletal mesh component
	USkeletalMeshComponent* GraspHelperOwnerSkelMC;

	// Manual override flags
	bool bGraspHelperCanExecuteManualOverrideFlag;
	/* End Ad Hoc Gras Helper */


	// Semantic data component of the owner
	USLIndividualComponent* IndividualComponent;

	// Semantic individual object
	USLBaseIndividual* OwnerIndividualObject;
	
	/* Grasp related */
	// Opposing group A for testing for grasps
	TArray<USLManipulatorBoneContactMonitor*> GroupA;

	// Opposing group B for testing for grasps
	TArray<USLManipulatorBoneContactMonitor*> GroupB;

	// Individuals currently grasped
	TSet<USLBaseIndividual*> GraspedIndividuals;

	// Active grasp type
	FString ActiveGraspType;
	
	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle GraspDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLGraspEndEvent> RecentlyEndedGraspEvents;

	/* Contact related */
	// Individuals in contact with group A
	TSet<USLBaseIndividual*> SetA;

	// Individuals in contact with group B
	TSet<USLBaseIndividual*> SetB;

	// Objects currently in contact and the number of shapes in contact with. Used of semantic contact detection
	TMap<USLBaseIndividual*, int32> IndividualsInContact;

	// Send finished events with a delay to check for possible concatenation of equal and consecutive events with small time gaps in between
	FTimerHandle ContactDelayTimerHandle;

	// Array of recently ended events
	TArray<FSLContactEndEvent> RecentlyEndedContactEvents;

	/* Constants */
	//static constexpr float MaxGraspEventTimeGap = 0.55f;
	//static constexpr float MaxContactEventTimeGap = 0.35f;
	static constexpr float ConcatenateIfSmallerDelay = 0.05f;
};
