// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLStructs.h" // FSLEntity
#include "SLContainerListener.generated.h"

/** Notify the beginning and the end of a opening/closing container event */
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLContainerManipulationSignature, const FSLEntity& /*Self*/, AActor* /*Container*/, float /*StartTime*/, float /*EndTime*/, const FString& /*Type*/);

/**
 * Checks for if the manipulated objects directly/indirectly opens / closes a container
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent) )
class USEMLOG_API USLContainerListener : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLContainerListener();

	// Dtor
	~USLContainerListener();

	// Check if owner is valid and semantically annotated
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

private:
	// Called on grasp begin
	void OnSLGraspBegin(const FSLEntity& Self, AActor* Other, float Time, const FString& GraspType);

	// Called on grasp end
	void OnSLGraspEnd(const FSLEntity& Self, AActor* Other, float Time);

	// Search which container will be manipulated and save their current distance to the grasped item
	bool SetContainersAndDistances();

	// Finish any active events
	void FinishActiveEvents();

public:
	// Container manipulation delegate
	FSLContainerManipulationSignature OnContainerManipulation;

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Semantic data of the owner
	FSLEntity SemanticOwner;

	// Object currently grasped
	AActor* CurrGraspedObj;

	// Containers being opened/closed (can be more, e.g. fridge handle opens the fridge and the fridge cladding)
	TArray<AActor*> Containers;

	/* Constants */
	constexpr static float MinDistance = 10.f;
};
