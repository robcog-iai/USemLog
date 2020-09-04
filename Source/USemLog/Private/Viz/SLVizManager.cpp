// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizManager.h"
#include "Viz/SLVizMarkerManager.h"
#include "Viz/SLVizHighlightMarkerManager.h"
#include "Viz/SLVizWorldManager.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLIndividualTypes.h"
#include "EngineUtils.h"

// Sets default values
ASLVizManager::ASLVizManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsInit = false;

	IndividualManager = nullptr;
	VizMarkerManager = nullptr;
	VizWorldManager = nullptr;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Dtor
ASLVizManager::~ASLVizManager()
{
	Reset();
}

// Called when the game starts or when spawned
void ASLVizManager::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLVizManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Button hacks */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bInitButtonHack))
	{
		bInitButtonHack = false;
		Init();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bHighlightButtonHack))
	{
		bHighlightButtonHack = false;
		HighlightIndividual(IndividualIdValueHack);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bRemoveHighlightButtonHack))
	{
		bRemoveHighlightButtonHack = false;
		RemoveIndividualHighlight(IndividualIdValueHack);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bRemoveAllHighlightsButtonHack))
	{
		bRemoveAllHighlightsButtonHack = false;
		RemoveAllIndividualHighlights();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bResetButtonHack))
	{
		bResetButtonHack = false;
		Reset();
	}
}

// Called when actor removed from game or game ended
void ASLVizManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Reset();
}
#endif // WITH_EDITOR

// Load all the required managers
bool ASLVizManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Viz manager (%s) is already init.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return true;
	}

	bool RetValue = true;

	if (!SetVizMarkerManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the viz marker manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}

	if (!SetVizHighlightMarkerManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the viz highligh marker manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}

	if (!SetVizWorldManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the viz world manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}

	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}
	if (!IndividualManager->Load(false))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		RetValue = false;
	}

	bIsInit = RetValue;	
	return RetValue;
}

// Clear any created markers / viz components
void ASLVizManager::Reset()
{
	RemoveAllIndividualHighlights();
}

// Highlight the individual (returns false if the individual is not found or is not of visual type)
bool ASLVizManager::HighlightIndividual(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (auto HM = HighlightedIndividuals.Find(Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is already highlighted.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(Id))
	{
		if (auto VI = Cast<USLVisibleIndividual>(Individual))
		{
			if (auto RI = Cast<USLRigidIndividual>(VI))
			{	
				HighlightedIndividuals.Add(Id,
					VizHighlightMarkerManager->CreateHighlightMarker(RI->GetStaticMeshComponent()));
				return true;
			}
			else if (auto SkI = Cast<USLSkeletalIndividual>(VI))
			{
				HighlightedIndividuals.Add(Id,
					VizHighlightMarkerManager->CreateHighlightMarker(SkI->GetSkeletalMeshComponent()));
				return true;
			}
			else if (auto BI = Cast<USLBoneIndividual>(VI))
			{
				HighlightedIndividuals.Add(Id,
					VizHighlightMarkerManager->CreateHighlightMarker(
						BI->GetSkeletalMeshComponent(),
						BI->GetMaterialIndex()));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is of unssuported visual type.."),
					*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of visible type, cannot highlight.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) cannot find individual (Id=%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}
	return false;
}

// Remove highlight from individual (returns false if the individual not found or it is not highlighted)
bool ASLVizManager::RemoveIndividualHighlight(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	USLVizHighlightMarker* HM = nullptr;
	if (HighlightedIndividuals.RemoveAndCopyValue(Id, HM))
	{
		VizHighlightMarkerManager->ClearMarker(HM);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) could not find individual (Id=%s) as highlighted.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}
}

// Remove all individual highlights
bool ASLVizManager::RemoveAllIndividualHighlights()
{
	if (!VizHighlightMarkerManager->IsValidLowLevel())
	{
		return false;
	}
		
	for (const auto& Pair : HighlightedIndividuals)
	{
		VizHighlightMarkerManager->ClearMarker(Pair.Value);
	}
	HighlightedIndividuals.Empty();
	return false;
}

// Get the vizualization marker manager from the world (or spawn a new one)
bool ASLVizManager::SetVizMarkerManager()
{
	if (VizMarkerManager->IsValidLowLevel() && !VizMarkerManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizMarkerManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			VizMarkerManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizMarkerManager");
	VizMarkerManager = GetWorld()->SpawnActor<ASLVizMarkerManager>(SpawnParams);
	VizMarkerManager->SetActorLabel(TEXT("SL_VizMarkerManager"));
	return true;
}

// Get the vizualization highlight marker manager from the world (or spawn a new one)
bool ASLVizManager::SetVizHighlightMarkerManager()
{
	if (VizHighlightMarkerManager->IsValidLowLevel() && !VizHighlightMarkerManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizHighlightMarkerManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			VizHighlightMarkerManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizHighlightMarkerManager");
	VizHighlightMarkerManager = GetWorld()->SpawnActor<ASLVizHighlightMarkerManager>(SpawnParams);
	VizHighlightMarkerManager->SetActorLabel(TEXT("SL_VizHighlightMarkerManager"));
	return true;
}

// Get the vizualization world manager from the world (or spawn a new one)
bool ASLVizManager::SetVizWorldManager()
{
	if (VizWorldManager->IsValidLowLevel() && !VizWorldManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizWorldManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			VizWorldManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizWorldManager");
	VizWorldManager = GetWorld()->SpawnActor<ASLVizWorldManager>(SpawnParams);
	VizWorldManager->SetActorLabel(TEXT("SL_VizWorldManager"));
	return true;
}

// Get the individual manager from the world (or spawn a new one)
bool ASLVizManager::SetIndividualManager()
{
	if (IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable())
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
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
	return true;
}
