// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SLRuntimeManager.h"
#include "SLFurnitureStateManager.generated.h"


/**
* Furniture states enum
*/
UENUM()
enum class EFurnitureState
{
	Closed			UMETA(DisplayName = "Closed"),
	HalfClosed		UMETA(DisplayName = "HalfClosed"),
	HalfOpened		UMETA(DisplayName = "HalfOpened"),
	Opened			UMETA(DisplayName = "Opened")	
};

/**
 * Manager for logging the states of the furnitures
 */
UCLASS()
class SEMLOG_API ASLFurnitureStateManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLFurnitureStateManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Init furniture states
	void InitStates();

	// Check all furniture states
	void CheckStates();

	// Get the current state of the furniture
	EFurnitureState GetState(AActor* FurnitureActor);

	// Start event
	void StartEvent(AActor* FurnitureActor, FOwlIndividualName Individual, EFurnitureState State);

	// Finish event
	void FinishEvent(AActor* FurnitureActor);

	// Update rate
	UPROPERTY(EditAnywhere, Category = SL)
	float UpdateRate;

	// Timer for checking the furniture state
	FTimerHandle FurnitureStateTimerHandle;

	// Semantic events runtime manager
	ASLRuntimeManager* SemLogRuntimeManager;

	// Drawers to initial position
	// needed because of linear constraint limitations:
	// https://answers.unrealengine.com/questions/450970/physics-constraint-linear-vs-angular-limitations.html
	TMap<AActor*, FVector> DrawerToInitLoc;

	// Drawer to constraint limit
	TMap<AActor*, float> DrawerToLimit;

	// Door to constraint
	TMap<AActor*, UPhysicsConstraintComponent*> DoorToConstraintComp;

	// Furniture to individual name
	TMap<AActor*, FOwlIndividualName> FurnitureToIndividual;

	// Furniture to state
	TMap<AActor*, EFurnitureState> FurnitureToState;

	// Map of actor furniture event individual
	TMap<AActor*, TSharedPtr<FOwlNode>> ActorToEvent;
};
