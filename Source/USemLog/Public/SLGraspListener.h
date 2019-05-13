// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLGraspOverlapShape.h"
#include "SLStructs.h" // FSLEntity
#include "SLGraspListener.generated.h"

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
 * Checks for physics based grasp events
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Grasp Listener")
class USEMLOG_API USLGraspListener : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLGraspListener();

	// Dtor
	~USLGraspListener();

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
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Load overlap groups, return true if at least one valid overlap is in each group
	bool LoadOverlapGroups();

private:
	// Check if the grasp trigger is active
	void InputAxisCallback(float Value);

	// Process beginning of contact in group A
	UFUNCTION()
	void OnBeginGroupAContact(AActor* OtherActor);

	// Process ending of contact in group A
	UFUNCTION()
	void OnEndGroupAContact(AActor* OtherActor);

	// Process beginning of contact in group B
	UFUNCTION()
	void OnBeginGroupBContact(AActor* OtherActor);

	// Process ending of contact in group B
	UFUNCTION()
	void OnEndGroupBContact(AActor* OtherActor);

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// The trigger for grasping is pulled
	bool bGraspingIsActive;
	
	// Semantic data of the owner
	FSLEntity SemanticOwner;

	// Read the input directly, avoid biding to various controllers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FName InputAxisName;

	// Opposing group A for testing for grasps
	TArray<USLGraspOverlapShape*> GroupA;

	// Opposing group B for testing for grasps
	TArray<USLGraspOverlapShape*> GroupB;

	// Objects in contact with group A
	TArray<AActor*> ItemsA;

	// Objects in contact with group B
	TArray<AActor*> ItemsB;
	
#if WITH_EDITOR
	// Hand type to load pre-defined parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLGraspHandType HandType;
#endif // WITH_EDITOR
};
