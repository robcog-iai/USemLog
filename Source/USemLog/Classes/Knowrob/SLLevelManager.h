// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLLevelManager.generated.h"

// Forward declarations
class ASLIndividualManager;
/**
 * 
 */
UCLASS()
class USEMLOG_API ASLLevelManager : public AInfo
{
    GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    ASLLevelManager();

public:
	// Load required manager
	void Init();

	// Check if the manager is initialized
	bool IsInit() const { return bIsInit; }

    // Switch the level (load another streaming level)
    void SwitchLevelTo(const FName& Map);

    // Print all the available level
    void GetAllLevel();

private:
    // Load the streaming level
    void LoadLevel(const FName& Level);
    
    // Unload the streaming level
    void UnloadLevel(const FName& Level);

    // Get the current visible streaming level
    FString GetVisibleLevel();

    // Remove PIE prefix and path
    FString RemoveAssetPathAndPrefix(const FString&  Asset);
	
	// Reset Individual manager after level is changed
	UFUNCTION(BlueprintCallable)
	void ResetIndividualManager();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// True if the manager is initialized
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	bool bIsInit;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;
};