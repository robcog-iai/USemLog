// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLContainerMonitor.generated.h"

// Forward declaration
class USLBaseIndividual;

/** Notify the beginning and the end of a opening/closing container event */
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLContainerManipulationSignature, USLBaseIndividual* /*Self*/, AActor* /*Container*/, float /*StartTime*/, float /*EndTime*/, const FString& /*Type*/);

/**
 * Checks for if the manipulated objects directly/indirectly opens / closes a container
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent) )
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
	void OnSLGraspBegin(USLBaseIndividual* Self, AActor* Other, float Time, const FString& GraspType);

	// Called on grasp end
	void OnSLGraspEnd(USLBaseIndividual* Self, AActor* Other, float Time);

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
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Semantic data component of the owner
	class USLIndividualComponent* IndividualComponent;

	// Semantic individual object
	class USLBaseIndividual* IndividualObject;

	// Object currently grasped
	AActor* CurrGraspedObj;

	// Grasp time
	float GraspTime;

	// Containers and their initial distance to the manipulator
	TMap<AActor*, float> ContainerToDistance;

	/* Constants */
	constexpr static float MinDistance = 5.f;
};

