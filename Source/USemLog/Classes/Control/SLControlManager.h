// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "HAL/ThreadSafeBool.h"

#include "SLControlManager.generated.h"


DECLARE_DELEGATE(FSLSimulationStart);
DECLARE_DELEGATE(FSLSimulationFinish);

// Forward declarations
class ASLIndividualManager;
/**
 * 
 */
UCLASS()
class USEMLOG_API ASLControlManager : public AInfo
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASLControlManager();
    
public:
	// Load required manager
    void Init();

	// Check if the manager if initialized
    bool IsInit() const { return bIsInit; }

	// Set the location and rotation of the individual
    bool SetIndividualPose(const FString& Id, FVector Location, FQuat Quat);
	
	// Apply force to individual
	bool ApplyForceTo(const FString& Id, FVector Force);

	// Apply physics simulation on individuals
    bool StartSimulationSelectionOnly(const TArray<FString>& Ids, int32 Seconds);

	// Stop physics simulation on individuals without delay
	bool StopSimulationSelectionOnly(const TArray<FString>& Ids);

	// Delegate for simulation start
	FSLSimulationFinish OnSimulationStart;

	// Delegate for simulation stop
	FSLSimulationFinish OnSimulationFinish;

private:
    // Get the individual manager from the world (or spawn a new one)
    bool SetIndividualManager();

    // True if the manager is initialized
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    bool bIsInit;

    // Keeps access to all the individuals in the world
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    ASLIndividualManager* IndividualManager;
};