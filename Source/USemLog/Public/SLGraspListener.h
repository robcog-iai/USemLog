// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLBoneOverlapShape.h"
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
* Skeletal type
*/
UENUM()
enum class ESLGraspSkeletalType : uint8
{
	Default					UMETA(DisplayName = "Default"),
	Genesis					UMETA(DisplayName = "Genesis"),
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

private:
	// Use a preconfigured set of names for the bones
	void AddDefaultParams();
#endif // WITH_EDITOR

	// Set overlap groups, return true if at least one valid overlap is in each group
	bool InitOverlapGroups();

	// Check if the grasp trigger is active
	void InputAxisCallback(float Value);

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

	// Bone names of overlap areas spawn location (at least one overlap needs to be active from both groups to have a grasp)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<FName> BoneNamesGroupA;

	// Bone names of overlap areas spawn location (at least one overlap needs to be active from both groups to have a grasp)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<FName> BoneNamesGroupB;

	UPROPERTY() // Avoid GC
	TArray<USLBoneOverlapShape*> GroupA;

	UPROPERTY() // Avoid GC
	TArray<USLBoneOverlapShape*> GroupB;
	
#if WITH_EDITOR
	// Hand type to add default bone types
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLGraspHandType HandType;

	// Hand type to add default bone types
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLGraspSkeletalType SkeletalType;

	// Mimic a button to add the default bone types
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bAddDefaultBoneTypes;
#endif // WITH_EDITOR
};
