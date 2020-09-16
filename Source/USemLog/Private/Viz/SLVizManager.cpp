// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizManager.h"

#include "Viz/SLVizMarkerManager.h"
#include "Viz/SLVizHighlightManager.h"
#include "Viz/SLVizEpisodeReplayManager.h"
#include "Individuals/SLIndividualManager.h"

#include "Individuals/Type/SLRigidIndividual.h"
#include "Individuals/Type/SLSkeletalIndividual.h"
#include "Individuals/Type/SLBoneIndividual.h"
#include "Individuals/Type/SLVirtualBoneIndividual.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLVizManager::ASLVizManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsInit = false;

	IndividualManager = nullptr;
	HighlightManager = nullptr;
	MarkerManager = nullptr;
	EpisodeReplayManager = nullptr;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLViz"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
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
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteInitButtonHack))
	{
		bExecuteInitButtonHack = false;
		Init();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteButtonHack))
	{
		bExecuteButtonHack = false;

		/* Highlight test */
		for (const auto& HT : HighlightTestValuesHack)
		{
			HighlightIndividual(HT.IndividualId, HT.Color, HT.MaterialType);
		}

		///* Primitive marker test */
		//for (const auto& PM : PrimitiveMarkerTestHack)
		//{
		//	CreatePrimitiveMarker(PM);
		//}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteUpdateButtonHack))
	{
		bExecuteUpdateButtonHack = false;

		/* Highlight test */
		for (const auto& HT : HighlightTestValuesHack)
		{
			UpdateIndividualHighlight(HT.IndividualId, HT.Color, HT.MaterialType);
		}

		///* Primitive marker test */
		//for (const auto& PM : PrimitiveMarkerTestHack)
		//{
		//	CreatePrimitiveMarker(PM);
		//}

	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteRemoveButtonHack))
	{
		bExecuteRemoveButtonHack = false;

		/* Highlight test */
		for (const auto& Id : RemoveTestHack)
		{
			RemoveIndividualHighlight(Id);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteRemoveAllButtonHack))
	{
		bExecuteRemoveAllButtonHack = false;

		/* Highlight test */
		RemoveAllIndividualHighlights();

	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteResetButtonHack))
	{
		bExecuteResetButtonHack = false;
		Reset();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizManager, bExecuteReplaySetupButtonHack))
	{
		bExecuteReplaySetupButtonHack = false;
		SetupWorldForEpisodeReplay();
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

	if (!SetVizHighlightManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the viz highligh marker manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}

	if (!SetVizMarkerManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the viz marker manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}

	if (!SetEpisodeReplayManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the viz world manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}
	bIsInit = RetValue;	
	return RetValue;
}

// Clear any created markers / viz components
void ASLVizManager::Reset()
{
	RemoveAllIndividualHighlights();
	IndividualManager = nullptr;
	HighlightManager = nullptr;
	MarkerManager = nullptr;
	EpisodeReplayManager = nullptr;
	bIsInit = false;
}


/* Highlights */
// Highlight the individual (returns false if the individual is not found or is not of visual type)
bool ASLVizManager::HighlightIndividual(const FString& Id, const FLinearColor& Color, ESLVizMaterialType MaterialType)
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
				UMeshComponent* MC = RI->GetStaticMeshComponent();
				HighlightManager->Highlight(MC, FSLVizVisualParams(Color, MaterialType));
				HighlightedIndividuals.Add(Id, FSLVizIndividualHighlightData(MC));
				return true;
			}
			else if (auto SkI = Cast<USLSkeletalIndividual>(VI))
			{
				UMeshComponent* MC = SkI->GetSkeletalMeshComponent();
				HighlightManager->Highlight(MC, FSLVizVisualParams(Color, MaterialType));
				HighlightedIndividuals.Add(Id, FSLVizIndividualHighlightData(MC));
				return true;
			}
			else if (auto BI = Cast<USLBoneIndividual>(VI))
			{
				UMeshComponent* MC = BI->GetSkeletalMeshComponent();
				int32 MaterialIndex = BI->GetMaterialIndex();
				HighlightManager->Highlight(MC, FSLVizVisualParams(Color, MaterialType, MaterialIndex));
				HighlightedIndividuals.Add(Id, FSLVizIndividualHighlightData(MC, MaterialIndex));
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

// Change the visual values of the highligted individual
bool ASLVizManager::UpdateIndividualHighlight(const FString& Id, const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (auto HighlightData = HighlightedIndividuals.Find(Id))
	{
		HighlightManager->UpdateHighlight(HighlightData->MeshComponent,
			FSLVizVisualParams(Color, MaterialType, HighlightData->MaterialSlots));
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) could not find individual (Id=%s) as highlighted.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
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

	FSLVizIndividualHighlightData HighlightData;
	if (HighlightedIndividuals.RemoveAndCopyValue(Id, HighlightData))
	{
		HighlightManager->ClearHighlight(HighlightData.MeshComponent);
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
void ASLVizManager::RemoveAllIndividualHighlights()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	for (const auto& HighlightDataPair : HighlightedIndividuals)
	{
		HighlightManager->ClearHighlight(HighlightDataPair.Value.MeshComponent);
	}
	HighlightedIndividuals.Empty();
}


/* Markers */
// Create a primitive marker
bool ASLVizManager::CreatePrimitiveMarker(const FString& MarkerId, 
	const TArray<FTransform>& Poses, 
	ESLVizPrimitiveMarkerType PrimitiveType,
	float Size, 
	const FLinearColor& Color, 
	ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp,Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Marker = MarkerManager->CreatePrimitiveMarker(Poses, PrimitiveType, Size, Color, MaterialType))
	{
		Markers.Add(MarkerId, Marker);
		return true;
	}

	return false;
}

// Create a marker by cloning the visual of the given individual (use original materials)
bool ASLVizManager::CreateStaticMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto RI = Cast<USLRigidIndividual>(Individual))
		{
			UStaticMesh* SM = RI->GetStaticMeshComponent()->GetStaticMesh();
			if (auto Marker = MarkerManager->CreateStaticMeshMarker(Poses, SM))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of rigid visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given individual
bool ASLVizManager::CreateStaticMeshMarker(const FString& MarkerId,
	const TArray<FTransform>& Poses,
	const FString& IndividualId,
	const FLinearColor& Color,
	ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto RI = Cast<USLRigidIndividual>(Individual))
		{
			UStaticMesh* SM = RI->GetStaticMeshComponent()->GetStaticMesh();
			if (auto Marker = MarkerManager->CreateStaticMeshMarker(Poses, SM, Color, MaterialType))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of rigid visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given skeletal individual (use original materials)
bool ASLVizManager::CreateSkeletalMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const TMap<int32, FTransform>& BonePoses, const FString& IndividualId)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
		{
			USkeletalMesh* SkelM = SkI->GetSkeletalMeshComponent()->SkeletalMesh;
			if (auto Marker = MarkerManager->CreateSkeletalMarker(Poses, SkelM))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given skeletal individual
bool ASLVizManager::CreateSkeletalMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const TMap<int32, FTransform>& BonePoses, const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
		{
			USkeletalMesh* SkelM = SkI->GetSkeletalMeshComponent()->SkeletalMesh;
			if (auto Marker = MarkerManager->CreateSkeletalMarker(Poses, SkelM, Color, MaterialType))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given individual (use original materials)
bool ASLVizManager::CreateBoneMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto BI = Cast<USLBoneIndividual>(Individual))
		{
			USkeletalMesh* SkelM = BI->GetSkeletalMeshComponent()->SkeletalMesh;
			int32 BoneIndex = BI->GetBoneIndex();
			int32 MaterialIndex = BI->GetMaterialIndex();
			TArray<TMap<int32, FTransform>> BonePosesArray;
			for (const auto& T : Poses)
			{
				TMap< int32, FTransform> Map;
				Map.Emplace(BoneIndex, T);
				BonePosesArray.Emplace(Map);
			}

			if (auto Marker = MarkerManager->CreateSkeletalMarker(Poses, SkelM,
				TArray<int32>{MaterialIndex}, BonePosesArray))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given individual
bool ASLVizManager::CreateBoneMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto BI = Cast<USLBoneIndividual>(Individual))
		{
			USkeletalMesh* SkelM = BI->GetSkeletalMeshComponent()->SkeletalMesh;
			int32 BoneIndex = BI->GetBoneIndex();
			int32 MaterialIndex = BI->GetMaterialIndex();
			TArray<TMap<int32, FTransform>> BonePosesArray;
			for (const auto& T : Poses)
			{
				TMap< int32, FTransform> Map;
				Map.Emplace(BoneIndex, T);
				BonePosesArray.Emplace(Map);
			}
			
			if (auto Marker = MarkerManager->CreateSkeletalMarker(Poses, SkelM, Color, MaterialType,
				TArray<int32>{MaterialIndex}, BonePosesArray))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Remove marker with the given id
bool ASLVizManager::RemoveMarker(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}



	return false;
}

// Remove all markers
void ASLVizManager::RemoveAllMarkers()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	for (const auto& Pair : Markers)
	{
		MarkerManager->ClearMarker(Pair.Value);
	}
	Markers.Empty();

	for (const auto& IdMarkerPair : Markers)
	{
		MarkerManager->ClearMarker(IdMarkerPair.Value);
	}
	Markers.Empty();
}


/* Episode replay */
// Setup the world for episode replay (remove physics, pause simulation, change skeletal meshes to poseable meshes)
bool ASLVizManager::SetupWorldForEpisodeReplay()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

#if WITH_EDITOR

	/*
	* World types info:
	* 
	* GIsEditor						-  True if we are in the editor. (True also when Play in Editor (PIE), or Simulating in Editor (SIE))
	* 
	* EWorldType::None				- An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels
	* EWorldType::Game				- The game world
	* EWorldType::Editor			- A world being edited in the editor
	* EWorldType::PIE				- A Play In Editor world
	* EWorldType::EditorPreview		- A preview world for an editor tool
	* EWorldType::GamePreview		- A preview world for a game
	* EWorldType::GameRPC			- A minimal RPC world for a game
	* EWorldType::Inactive			- An editor world that was loaded but not currently being edited in the level editor
	* 
	* GEditor->PlayWorld				- A pointer to a UWorld that is the duplicated/saved-loaded to be played in with "Play From Here"
	* GEditor->bIsSimulatingInEditor	- True if we're Simulating In Editor, as opposed to Playing In Editor.  In this mode, simulation takes place right the level editing environment
	* GIsPlayInEditorWorld				- Whether GWorld points to the play in editor world
	* GWorld->HasBegunPlay()			- True if gamplay has started
	*/

	// If we are running in the editor, make sure we are running a duplicated world (not the editor active one)
	if (GIsEditor && !GEditor->PlayWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d we are in editor world, this will not be set up for replay.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
#endif // WITH_EDITOR

	EpisodeReplayManager->SetWorldAsVisualOnly();
	return EpisodeReplayManager->IsWorldSetASVisualOnly();
}

// Change the data into an episode format and load it to the episode replay manager
void ASLVizManager::LoadEpisodeData(const TArray<TPair<float, TMap<FString, FTransform>>>& InCompactEpisodeData)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!EpisodeReplayManager->IsWorldSetASVisualOnly())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) cannot load episode data because the world is not set as visual only.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (InCompactEpisodeData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) the episode data is empty.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Create and reserve episode data with the array size
	FSLVizEpisodeData EpisodeData(InCompactEpisodeData.Num());

	// Process first frame (contains all individuals -- the rest of the frames contain only individuals that have moved)
	FSLVizEpisodeFrameData FrameData;
	for (const auto& IndividualPosePair : InCompactEpisodeData[0].Value)
	{
		if (auto Individual = IndividualManager->GetIndividual(IndividualPosePair.Key))
		{
			if (auto RI = Cast<USLRigidIndividual>(Individual))
			{
				FrameData.ActorPoses.Emplace(RI->GetParentActor(), IndividualPosePair.Value);
			}
			else if (auto BI = Cast<USLBoneIndividual>(Individual))
			{

			}
			else if (auto VBI = Cast<USLVirtualBoneIndividual>(Individual))
			{

			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) could not find individual with id=%s, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualPosePair.Key);
		}		
	}
	//EpisodeDataRaw.Timestamps[0] = InCompactEpisodeData[0].Key;
	EpisodeData.Timestamps.Emplace(InCompactEpisodeData[0].Key);
	//EpisodeDataRaw.Frames[0] = FrameData;
	EpisodeData.Frames.Emplace(FrameData);
	
	// Process the following frames
	for (int32 FrameIndex = 1; FrameIndex < InCompactEpisodeData.Num(); ++FrameIndex)
	{
		//FSLVizEpisodeFrameData CurrFrameData = FrameData;
		for (const auto& IndividualPosePair : InCompactEpisodeData[FrameIndex].Value)
		{
			if (auto Individual = IndividualManager->GetIndividual(IndividualPosePair.Key))
			{
				if (auto RI = Cast<USLRigidIndividual>(Individual))
				{
					if (auto FoundActorPose = FrameData.ActorPoses.Find(RI->GetParentActor()))
					{
						(*FoundActorPose) = IndividualPosePair.Value;
					}
				}
				else if (auto BI = Cast<USLBoneIndividual>(Individual))
				{

				}
				else if (auto VBI = Cast<USLVirtualBoneIndividual>(Individual))
				{

				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager (%s) could not find individual with id=%s, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualPosePair.Key);
			}
		}
	}
}




/* Managers */
// Get the individual manager from the world (or spawn a new one)
bool ASLVizManager::SetIndividualManager()
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
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
	return true;
}

// Get the vizualization highlight manager from the world (or spawn a new one)
bool ASLVizManager::SetVizHighlightManager()
{
	if (HighlightManager && HighlightManager->IsValidLowLevel() && !HighlightManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizHighlightManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			HighlightManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizHighlightManager");
	HighlightManager = GetWorld()->SpawnActor<ASLVizHighlightManager>(SpawnParams);
	HighlightManager->SetActorLabel(TEXT("SL_VizHighlightManager"));
	return true;
}

// Get the vizualization marker manager from the world (or spawn a new one)
bool ASLVizManager::SetVizMarkerManager()
{
	if (MarkerManager && MarkerManager->IsValidLowLevel() && !MarkerManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizMarkerManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			MarkerManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizMarkerManager");
	MarkerManager = GetWorld()->SpawnActor<ASLVizMarkerManager>(SpawnParams);
	MarkerManager->SetActorLabel(TEXT("SL_VizMarkerManager"));
	return true;
}

// Get the vizualization episode replay manager from the world (or spawn a new one)
bool ASLVizManager::SetEpisodeReplayManager()
{
	if (EpisodeReplayManager && EpisodeReplayManager->IsValidLowLevel() && !EpisodeReplayManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizEpisodeReplayManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			EpisodeReplayManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_EpisodeReplayManager");
	EpisodeReplayManager = GetWorld()->SpawnActor<ASLVizEpisodeReplayManager>(SpawnParams);
	EpisodeReplayManager->SetActorLabel(TEXT("SL_EpisodeReplayManager"));
	return true;
}
