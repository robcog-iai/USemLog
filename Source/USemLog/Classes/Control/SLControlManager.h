// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLControlManager.generated.h"

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
    bool Init();

    bool IsInit() const { return bIsInit; }

    void SetIndividualPose(const FString& Id, FVector Location, FQuat Quat);

    void StartSimulationSelectionOnly(const TArray<FString>& Ids);

    void StopSimulationSelectionOnly(const TArray<FString>& Ids);
    
private:
    // Get the individual manager from the world (or spawn a new one)
    bool SetIndividualManager();

private:
    // True if the manager is initialized
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    bool bIsInit;

    // Keeps access to all the individuals in the world
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    ASLIndividualManager* IndividualManager;

};