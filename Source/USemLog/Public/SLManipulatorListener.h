// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLManipulatorOverlapSphere.h"
#include "SLStructs.h" // FSLEntity
#include "SLContactBox.h" // semantic contact delegates
#include "SLManipulatorListener.generated.h"

/**
* Hand type
*/
UENUM()
enum class ESLGraspHandType : uint8
{
	Left					UMETA(DisplayName = "Left"),
	Right					UMETA(DisplayName = "Right"),
};

/** Notify when an object is grasped */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FBeginSLManipulatorOverlapSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);

/** Notify when an object is released */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FEndSLManipulatorEndSignature, const FSLEntity& /*Self*/, UObject* /*Other*/, float /*Time*/);

/**
 * Checks for physics based grasp events and semantic contacts
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Manipulator Listener")
class USEMLOG_API USLManipulatorListener : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLManipulatorListener();

	// Dtor
	~USLManipulatorListener();

	// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
	bool Init(bool bInDetectGrasps, bool bInDetectContacts);

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

private:
	// Pause/continue the grasp detection
	void Pause(bool bInPause);

	// Check if the grasp trigger is active
	void InputAxisCallback(float Value);

	// Grasp update check
	void CheckGraspState();

	// A grasp has started
	void BeginGrasp(AActor* Other);
	
	// A grasp has ended
	void EndGrasp(AActor* Other);

	// All grasps have ended
	void EndAllGrasps();

	// Process beginning of grasp related contact in group A
	UFUNCTION()
	void OnBeginGroupAGraspContact(AActor* OtherActor);

	// Process beginning of grasp related contact in group B
	UFUNCTION()
	void OnBeginGroupBGraspContact(AActor* OtherActor);

	// Process beginning of contact
	UFUNCTION()
	void OnBeginContact(AActor* OtherActor);

	// Process ending of grasp related contact in group A
	UFUNCTION()
	void OnEndGroupAGraspContact(AActor* OtherActor);

	// Process ending of grasp related  contact in group B
	UFUNCTION()
	void OnEndGroupBGraspContact(AActor* OtherActor);

	// Process ending of contact
	UFUNCTION()
	void OnEndContact(AActor* OtherActor);
	
public:
	// Event called when grasp occurs
	FBeginSLManipulatorOverlapSignature OnBeginManipulatorGrasp;

	// Event called when grasp ends
	FEndSLManipulatorEndSignature OnEndManipulatorGrasp;

	// Event called when a semantic overlap begins
	FBeginSLContactSignature OnBeginManipulatorContact;

	// Event called when a semantic overlap ends
	FEndSLContactSignature OnEndManipulatorContact;

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// True grasp detection is paused
	bool bIsPaused;

	// New information added 
	bool bGraspIsDirty;

	// Detect grasp contacts
	bool bDetectGrasps;

	// Detect contacts
	bool bDetectContacts;
	
#if WITH_EDITOR
	// Hand type to load pre-defined parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLGraspHandType HandType;
#endif // WITH_EDITOR
	
	// Read the input directly, avoid biding to various controllers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName InputAxisName;

	// Axis input value to wake up from idle
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UnPauseTriggerVal;

	// If the owner is not a skeletal actor, one needs to add the children (fingers) manually
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bIsNotSkeletal;

	// Explicit reference to the children (fingers) if the owner is not a skeletal actor
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bIsNotSkeletal"))
	TArray<AStaticMeshActor*> Fingers;

	// Semantic data of the owner
	FSLEntity SemanticOwner;

	// Opposing group A for testing for grasps
	TArray<USLManipulatorOverlapSphere*> GroupA;

	// Opposing group B for testing for grasps
	TArray<USLManipulatorOverlapSphere*> GroupB;

	// Objects in contact with group A
	TSet<AActor*> SetA;

	// Objects in contact with group B
	TSet<AActor*> SetB;

	// Objects currently grasped
	TArray<AActor*> GraspedObjects;

	// Objects currently in contact and the number of shapes in contact with. Used of semantic contact detection
	TMap<AActor*, int32> ContactObjects;
};
