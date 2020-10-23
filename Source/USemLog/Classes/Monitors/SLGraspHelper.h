// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "SLGraspHelper.generated.h"

// Forward declaration
class USLManipulatorMonitor;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPhysicsConstraintComponent;
class UWorld;

/* Rotation control type of the hands */
UENUM(/*BlueprintType*/)
enum class ESLGraspHelperType : uint8
{
	Default					UMETA(DisplayName = "Default"),
	ABGroup					UMETA(DisplayName = "ABGroup"),
	AllBones	     		UMETA(DisplayName = "AllBones")
};

/**
* Grasp helper synced with the grasp detection algorithm
*/
USTRUCT(/*BlueprintType*/)
struct USEMLOG_API FSLGraspHelper
{
	GENERATED_BODY()

public:
	// Default constructor
	FSLGraspHelper() {};

	// Init grasp helper
	void Init(USLManipulatorMonitor* ManipulatorMonitor);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Start grasp help
	void StartGraspHelp();

	// End grasp help
	void EndGraspHelp();

	// Check is the grasp help should be started on the given actor
	void CheckStartGraspHelp(AActor* Actor);

	// Check is the grasp help should be ended on the given actor
	void CheckEndGraspHelp(AActor* Actor);

	// Set group A bone name
	void SetBoneNameGroupA(const FName& BoneName);

	// Clear group A bone name
	void ClearBoneNameGroupA();

	// Set group B bone name
	void SetBoneNameGroupB(const FName& BoneName);

	// Clear group B bone name
	void ClearBoneNameGroupB();
		
private:
	// Delayd all for start help
	void DelayedStartHelp();

	// Create and register the grasp helper constraint
	UPhysicsConstraintComponent* CreateGraspHelperConstraint(const FName& Name);

	// Check if actor can/should be grasped
	bool ShouldBeGrasped(AActor* Actor) const;

	/* Debug helper functions */
	// Get owner actor name
	FString GetOwnerActorName() const;

public:
	// Log various debug output messages
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogDebug = false;

	// Grasp type (one constraint, two - one from each group, or for all bones
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLGraspHelperType GraspHelpType = ESLGraspHelperType::Default;

	// Start helping the grasp after a delay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bDelayGrasp = true;

	// Decrease mass of grasped object
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bDelayGrasp"))
	float GraspDelayValue = 0.25f;

	// Attach default constraint to the given bone
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName DefaultBoneName;

	// Start helping the grasp after a delay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseUserInput = false;

	// Input action name in case manual override is used
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseUserInput"))
	FName InputActionName;

	// Disable gravity on grasp
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bDisableGravityOnGraspedSMC;

	// Decrease mass of object
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bScaleMassOnGraspedSMC;

	// Decrease mass of grasped object
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bScaleMassOnGraspedIndividual"))
	float ScaleMassValue = 0.1f;

	// Constraint movement limit (all axis)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Constraint")
	float ConstraintLimit;

	// Grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Constraint")
	float ConstraintStiffness;

	// Grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Constraint")
	float ConstraintDamping;

	// Grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Constraint")
	float ConstraintContactDistance;

	// Grasp helper constraint properties
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Constraint")
	bool bConstraintParentDominates;

private:
	// True if initialized
	bool bIsInit = false;

	// True if the grasp is currently active
	bool bIsGraspHelpActive = false;

	// Enabled user input based grasp start/stop execution
	bool bWaitingForUserInputEnabled = false;

	// Pointer to owner manipulator monitor
	USLManipulatorMonitor* OwnerMM;

	// Pointer to world
	UWorld* World;

	// Grasped mesh component
	UStaticMeshComponent* GraspedStaticMeshComp;

	// Hand skeletal mesh component
	USkeletalMeshComponent* HandSkelMeshComp;

	// Grasp helper trigger delay
	FTimerHandle DelayTimerHandle;

	// Timer delegate to be able to bind against non UObject functions
	FTimerDelegate TimerDelegate;

	// Grasp helper constraint component
	UPROPERTY()
	UPhysicsConstraintComponent* DefaultConstraint = nullptr;

	// Constraint attached to a group A bone
	UPROPERTY()
	UPhysicsConstraintComponent* ConstraintGroupA = nullptr;

	// Constraint attached to a group B bone
	UPROPERTY()
	UPhysicsConstraintComponent* ConstraintGroupB = nullptr;

	// Constraints attached to all the bones
	UPROPERTY()
	TMap<UPhysicsConstraintComponent*, FName> AllBonesConstraints;

	// Bone name from group A
	FName BoneNameGroupA = NAME_None;

	// Bone name from group A
	FName BoneNameGroupB = NAME_None;


};

