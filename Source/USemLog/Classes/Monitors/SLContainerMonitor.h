// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLContainerMonitor.generated.h"

// Forward declaration
class USLBaseIndividual;

/** Notify the beginning and the end of a opening/closing container event */
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLContainerManipulationSignature, USLBaseIndividual* /*Self*/, USLBaseIndividual* /*Container*/, float /*StartTime*/, float /*EndTime*/, const FString& /*Type*/);

/**
 * Checks for if the manipulated objects directly/indirectly opens / closes a container
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Container Monitor")
class USEMLOG_API USLContainerMonitor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLContainerMonitor();

	// Dtor
	~USLContainerMonitor();

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
	void OnSLGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& GraspType);

	// Called on grasp end
	void OnSLGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);

	// Search which container will be manipulated and save their current distance to the grasped item
	bool SetContainersAndDistances();

	// Finish any active events
	void FinishActiveEvents();

	// Iterate recursively attached constraints actors of parent, append other constrained actors to set
	void GetAllConstraintsOtherActors(AActor* Actor, TSet<AActor*>& OutOtherConstraintActors);

	// Iterate recursively on the attached actors, and search for container type
	void GetAllAttachedContainers(AActor* Actor, TSet<AActor*>& OutContainers);

public:
	// Container manipulation delegate
	FSLContainerManipulationSignature OnContainerManipulation;

private:
	// True if initialized
	uint8 bIsInit : 1;

	// True if started
	uint8 bIsStarted : 1;

	// True if finished
	uint8 bIsFinished : 1;

	// Semantic data component of the owner
	class USLIndividualComponent* OwnerIndividualComponent;

	// Semantic individual object
	USLBaseIndividual* OwnerIndividualObject;

	// Object currently grasped
	USLBaseIndividual* CurrGraspedIndividual;

	// Grasp time
	float GraspTime;

	// Containers and their initial distance to the manipulator
	TMap<AActor*, float> ContainerToDistance;

	/* Constants */
	constexpr static float MinDistance = 5.f;
};

