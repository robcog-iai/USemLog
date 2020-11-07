// Fill out your copyright notice in the Description page of Project Settings.


#include "Knowrob/SLSemanticMapManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLBaseIndividual.h"

ASLSemanticMapManager::ASLSemanticMapManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

bool ASLSemanticMapManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Semantic Map manager (%s) is already init.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return true;
	}

	bool RetValue = true;
	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Semantic Map manager (%s) could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}
	if (!IndividualManager->Load(false))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Semantic Map (%s) could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		RetValue = false;
	}
	bIsInit = RetValue;
	return RetValue;
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
    LatentInfo.UUID = 1;
	LatentInfo.CallbackTarget = this; 
	LatentInfo.ExecutionFunction = FName(TEXT("ResetIndividualManager"));
	LatentInfo.Linkage = 0;
    UGameplayStatics::LoadStreamLevel(this, Level, true, false, LatentInfo);
}

// Unload the streaming level
void ASLSemanticMapManager::UnloadLevel(const FName& Level)
{
    FLatentActionInfo LatentInfo;
    LatentInfo.UUID = 2;
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

// Reset Individual manager after level is changed
void ASLSemanticMapManager::ResetIndividualManager()
{
	IndividualManager->Init(true);
	IndividualManager->Load(true);
}

// Get the individual manager from the world (or spawn a new one)
bool ASLSemanticMapManager::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLIndividualManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			IndividualManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_IndividualManager");
	IndividualManager = GetWorld()->SpawnActor<ASLIndividualManager>(SpawnParams);
#if WITH_EDITOR
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
	return true;
}