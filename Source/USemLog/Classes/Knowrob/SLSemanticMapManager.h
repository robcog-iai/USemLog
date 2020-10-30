// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLSemanticMapManager.generated.h"

/**
 * 
 */
UCLASS()
class USEMLOG_API ASLSemanticMapManager : public AInfo
{
    GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    ASLSemanticMapManager();

public:
    // Load the Semantic Map
    void LoadMap(const FName& Map);

    // Print all the available Semantic map
    void GetAllMaps();

private:
    // Load the streaming level
    void LoadLevel(const FName& Level);
    
    // Unload the streaming level
    void UnloadLevel(const FName& Level);

    // Get the current visible streaming level
    FString GetVisibleLevel();

    // Remove PIE prefix and path
    FString RemoveAssetPathAndPrefix(const FString&  Asset);
};