// Fill out your copyright notice in the Description page of Project Settings.


#include "Knowrob/SLSemanticMapManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"

ASLSemanticMapManager::ASLSemanticMapManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Load the Semantic Map
void ASLSemanticMapManager::LoadMap(const FName& Map) 
{
    FString VisibleLevel = GetVisibleLevel();
    if (!VisibleLevel.Equals("None") && VisibleLevel.Equals(Map.ToString()))
    {
        return;
    }
    
    if (!VisibleLevel.Equals("None"))
    {
        UnloadLevel(FName(*VisibleLevel));
    }
     
    LoadLevel(Map);
}

// Print all the available Semantic map
void ASLSemanticMapManager::GetAllMaps()
{
    const TArray<ULevelStreaming*>& StreamedLevels = GetWorld()->GetStreamingLevels();

    for (const ULevelStreaming* EachLevelStreaming : StreamedLevels)
    {
        if (!EachLevelStreaming)
        {
            continue;
        }

        ULevel* EachLevel = EachLevelStreaming->GetLoadedLevel();

        UE_LOG(LogTemp, Warning, TEXT("Available Semantic Maps: %s"), *RemoveAssetPathAndPrefix(EachLevelStreaming->GetWorldAssetPackageName()));
    }
}

// Load the streaming level
void ASLSemanticMapManager::LoadLevel(const FName& Level)
{
    FLatentActionInfo LatentInfo;
    LatentInfo.UUID = 0;
    UGameplayStatics::LoadStreamLevel(this, Level, true, false, LatentInfo);
}

// Unload the streaming level
void ASLSemanticMapManager::UnloadLevel(const FName& Level)
{
    FLatentActionInfo LatentInfo;
    LatentInfo.UUID = 1;
    UGameplayStatics::UnloadStreamLevel(this, Level, LatentInfo, false);
}

// Get the current visible streaming level
FString ASLSemanticMapManager::GetVisibleLevel()
{
    const TArray<ULevelStreaming*>& StreamedLevels = GetWorld()->GetStreamingLevels();

    for (const ULevelStreaming* EachLevelStreaming : StreamedLevels)
    {
        if (!EachLevelStreaming)
        {
            continue;
        }

        ULevel* EachLevel = EachLevelStreaming->GetLoadedLevel();
    
        //Is This Level Valid and Visible?
        if (EachLevel && EachLevel->bIsVisible)
        {
            return RemoveAssetPathAndPrefix(EachLevelStreaming->GetWorldAssetPackageName());
        }
    }
    return TEXT("None");
}

// Remove PIE prefix and path
FString ASLSemanticMapManager::RemoveAssetPathAndPrefix(const FString&  Asset)
{
    TArray<FString> AssetPathArr;
    GetWorld()->RemovePIEPrefix(Asset).ParseIntoArray(AssetPathArr, TEXT("/"), true);
    return AssetPathArr.Last();
}