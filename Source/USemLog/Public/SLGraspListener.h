// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLBoneOverlapShape.h"
#include "SLStructs.h" // FSLEntity
#include "SLGraspListener.generated.h"

/**
 * Checks for physics based grasp events
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent) )
class USEMLOG_API USLGraspListener : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLGraspListener();

	// Dtor
	~USLGraspListener();

	// Initialize trigger areas for runtime, check if owner is valid and semantically annotated
	void Init();

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

private:
	// Set overlap groups, return true if at least one valid overlap is in each group
	bool SetOverlapGroups();

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;
	
	// Semantic data of the owner
	FSLEntity SemanticOwner;

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

};
