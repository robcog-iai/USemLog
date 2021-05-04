// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizManager.h"

#include "Viz/SLVizMarkerManager.h"
#include "Viz/SLVizHighlightManager.h"
//#include "Viz/SLVizEpisodeManager.h"
#include "Viz/SLVizEpisodeUtils.h"
#include "Viz/SLVizCameraDirector.h"
#include "Individuals/SLIndividualManager.h"

#include "Individuals/Type/SLRigidIndividual.h"
#include "Individuals/Type/SLSkeletalIndividual.h"
#include "Individuals/Type/SLBoneIndividual.h"
#include "Individuals/Type/SLVirtualBoneIndividual.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"


#if WITH_EDITOR
#include "Editor.h"	// GEditor
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
	EpisodeManager = nullptr;

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

// Called when actor removed from game or game ended
void ASLVizManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Reset();
}

// Load all the required managers
void ASLVizManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s is already init.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!IndividualManager->IsLoaded() && !IndividualManager->Load(true))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		return;
	}

	if (!SetVizHighlightManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the viz highligh marker manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!SetVizMarkerManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the viz marker manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!SetEpisodeManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the viz world manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!SetCameraDirector())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not set the viz camera director.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	bIsInit = true;	
}

// Clear any created markers / viz components
void ASLVizManager::Reset()
{
	RemoveAllIndividualHighlights();
	IndividualManager = nullptr;
	HighlightManager = nullptr;
	MarkerManager = nullptr;
	EpisodeManager = nullptr;
	bIsInit = false;
	CachedEpisodeData.Empty();
}


/* Highlights */
// Highlight the individual (returns false if the individual is not found or is not of visual type)
bool ASLVizManager::HighlightIndividual(const FString& Id, const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (auto HD = HighlightedIndividuals.Find(Id))
	{
		HighlightManager->UpdateHighlight(HD->MeshComponent,
			FSLVizVisualParams(Color, MaterialType, HD->MaterialSlots));
		return true;
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is already highlighted.."),
		//	*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		//return false;
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
				UMeshComponent* MC = SkI->GetVisibleMeshComponent();
				HighlightManager->Highlight(MC, FSLVizVisualParams(Color, MaterialType));
				HighlightedIndividuals.Add(Id, FSLVizIndividualHighlightData(MC));
				return true;
			}
			else if (auto BI = Cast<USLBoneIndividual>(VI))
			{
				UMeshComponent* MC = BI->GetVisibleMeshComponent();
				int32 MaterialIndex = BI->GetMaterialIndex();
				HighlightManager->Highlight(MC, FSLVizVisualParams(Color, MaterialType, MaterialIndex));
				HighlightedIndividuals.Add(Id, FSLVizIndividualHighlightData(MC, MaterialIndex));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is of unssuported visual type.."),
					*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of visible type, cannot highlight.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s cannot find individual (Id=%s).."),
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (auto HD = HighlightedIndividuals.Find(Id))
	{
		HighlightManager->UpdateHighlight(HD->MeshComponent,
			FSLVizVisualParams(Color, MaterialType, HD->MaterialSlots));
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s could not find individual (Id=%s) as highlighted.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
	return false;
}

// Remove highlight from individual (returns false if the individual not found or it is not highlighted)
bool ASLVizManager::RemoveIndividualHighlight(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s could not find individual (Id=%s) as highlighted.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}
}

// Remove all individual highlights
void ASLVizManager::RemoveAllIndividualHighlights()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	for (const auto& HighlightDataPair : HighlightedIndividuals)
	{
		HighlightManager->ClearHighlight(HighlightDataPair.Value.MeshComponent);
	}
	HighlightedIndividuals.Empty();
}

// Spawn or get manager from the world
ASLVizManager* ASLVizManager::GetExistingOrSpawnNew(UWorld* World)
{
	// Check in world
	for (TActorIterator<ASLVizManager>Iter(World); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			return *Iter;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizManager");
	auto Manager = World->SpawnActor<ASLVizManager>(SpawnParams);
#if WITH_EDITOR
	Manager->SetActorLabel(TEXT("SL_VizManager"));
#endif // WITH_EDITOR
	return Manager;
}


/* Markers */
/* Primitive */
// Create a primitive marker
bool ASLVizManager::CreatePrimitiveMarker(const FString& MarkerId, 	const TArray<FTransform>& Poses, 
	ESLVizPrimitiveMarkerType PrimitiveType,
	float Size, 
	const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp,Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
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

// Create a primitive marker timeline
bool ASLVizManager::CreatePrimitiveMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
	ESLVizPrimitiveMarkerType PrimitiveType,
	float Size,
	const FLinearColor& Color, ESLVizMaterialType MaterialType,
	const FSLVizTimelineParams& TimelineParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Marker = MarkerManager->CreatePrimitiveMarkerTimeline(Poses, PrimitiveType, Size,
		Color, MaterialType, TimelineParams))
	{
		Markers.Add(MarkerId, Marker);
		return true;
	}

	return false;
}


/* Static mesh */
// Create a marker by cloning the visual of the given individual (use original materials)
bool ASLVizManager::CreateStaticMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
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
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of rigid visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given individual
bool ASLVizManager::CreateStaticMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId,
	const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
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
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of rigid visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a timeline marker by cloning the visual of the given individual (use original materials)
bool ASLVizManager::CreateStaticMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId,
	const FSLVizTimelineParams& TimelineParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto RI = Cast<USLRigidIndividual>(Individual))
		{
			UStaticMesh* SM = RI->GetStaticMeshComponent()->GetStaticMesh();
			if (auto Marker = MarkerManager->CreateStaticMeshMarkerTimeline(Poses, SM, TimelineParams))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of rigid visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a timeline marker by cloning the visual of the given individual
bool ASLVizManager::CreateStaticMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
	const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType,
	const FSLVizTimelineParams& TimelineParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto RI = Cast<USLRigidIndividual>(Individual))
		{
			UStaticMesh* SM = RI->GetStaticMeshComponent()->GetStaticMesh();
			if (auto Marker = MarkerManager->CreateStaticMeshMarkerTimeline(Poses, SM, Color, MaterialType, TimelineParams))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of rigid visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}


/* Skeletal mesh */
// Create a marker by cloning the visual of the given skeletal individual (use original materials)
bool ASLVizManager::CreateSkeletalMeshMarker(const FString& MarkerId,
	const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	const FString& IndividualId)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
		{
			USkeletalMesh* SkelM = SkI->GetSkeletalMeshComponent()->SkeletalMesh;
			if (auto Marker = MarkerManager->CreateSkeletalMarker(SkeletalPoses, SkelM))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given skeletal individual
bool ASLVizManager::CreateSkeletalMeshMarker(const FString& MarkerId,
	const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
		{
			USkeletalMesh* SkelM = SkI->GetSkeletalMeshComponent()->SkeletalMesh;
			if (auto Marker = MarkerManager->CreateSkeletalMarker(SkeletalPoses, SkelM, Color, MaterialType))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a timeline by cloning the visual of the given skeletal individual (use original materials)
bool ASLVizManager::CreateSkeletalMeshMarkerTimeline(const FString& MarkerId, const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	const FString& IndividualId, const FSLVizTimelineParams& TimelineParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
		{
			USkeletalMesh* SkelM = SkI->GetSkeletalMeshComponent()->SkeletalMesh;
			if (auto Marker = MarkerManager->CreateSkeletalMarkerTimeline(SkeletalPoses, SkelM,
				TimelineParams))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a timeline by cloning the visual of the given skeletal individual
bool ASLVizManager::CreateSkeletalMeshMarkerTimeline(const FString& MarkerId, const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType,
	const FSLVizTimelineParams& TimelineParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
		{
			USkeletalMesh* SkelM = SkI->GetSkeletalMeshComponent()->SkeletalMesh;
			if (auto Marker = MarkerManager->CreateSkeletalMarkerTimeline(SkeletalPoses, SkelM,
				Color, MaterialType, TimelineParams))
			{
				Markers.Add(MarkerId, Marker);
				return true;
			};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto BI = Cast<USLBoneIndividual>(Individual))
		{
			//USkeletalMesh* SkelM = BI->GetSkeletalMeshComponent()->SkeletalMesh;
			//int32 BoneIndex = BI->GetBoneIndex();
			//int32 MaterialIndex = BI->GetMaterialIndex();
			//TArray<TMap<int32, FTransform>> BonePosesArray;
			//for (const auto& T : Poses)
			//{
			//	TMap< int32, FTransform> Map;
			//	Map.Emplace(BoneIndex, T);
			//	BonePosesArray.Emplace(Map);
			//}

			//if (auto Marker = MarkerManager->CreateSkeletalMarker(Poses, SkelM,
			//	BonePosesArray, TArray<int32>{MaterialIndex}))
			//{
			//	Markers.Add(MarkerId, Marker);
			//	return true;
			//};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a marker by cloning the visual of the given individual
bool ASLVizManager::CreateBoneMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType)
{
	// TODO
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO "), *FString(__FUNCTION__), __LINE__);

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (Markers.Contains(MarkerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s marker (Id=%s) already exists.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MarkerId);
		return false;
	}

	if (auto Individual = IndividualManager->GetIndividual(IndividualId))
	{
		if (auto BI = Cast<USLBoneIndividual>(Individual))
		{
			//USkeletalMesh* SkelM = BI->GetSkeletalMeshComponent()->SkeletalMesh;
			//int32 BoneIndex = BI->GetBoneIndex();
			//int32 MaterialIndex = BI->GetMaterialIndex();
			//TArray<TMap<int32, FTransform>> BonePosesArray;
			//for (const auto& T : Poses)
			//{
			//	TMap< int32, FTransform> Map;
			//	Map.Emplace(BoneIndex, T);
			//	BonePosesArray.Emplace(Map);
			//}
			//
			//if (auto Marker = MarkerManager->CreateSkeletalMarker(Poses, SkelM, Color, MaterialType,
			//	BonePosesArray, TArray<int32>{MaterialIndex}))
			//{
			//	Markers.Add(MarkerId, Marker);
			//	return true;
			//};
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s individual (Id=%s) is not of skeletal visible type, cannot create a clone marker.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualId);
			return false;
		}
	}
	return false;
}

// Create a timeline marker by cloning the visual of the given individual
bool ASLVizManager::CreateBoneMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId, const FSLVizTimelineParams& TimelineParams)
{
	// TODO
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO "), *FString(__FUNCTION__), __LINE__);
	return false;
}

// Create a timeline marker by cloning the visual of the given individual
bool ASLVizManager::CreateBoneMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses, const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType, const FSLVizTimelineParams& TimelineParams)
{
	// TODO
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO "), *FString(__FUNCTION__), __LINE__);
	return false;
}


// Remove marker with the given id
bool ASLVizManager::RemoveMarker(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	USLVizBaseMarker* RemovedMarker = nullptr;
	if (Markers.RemoveAndCopyValue(Id, RemovedMarker))
	{
		MarkerManager->ClearMarker(RemovedMarker);
		return true;
	}
	return false;
}

// Remove all markers
void ASLVizManager::RemoveAllMarkers()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	for (const auto& IdMarkerPair : Markers)
	{
		MarkerManager->ClearMarker(IdMarkerPair.Value);
	}
	Markers.Empty();
}


/* Episode replay */
// Setup the world for episode replay (remove physics, pause simulation, change skeletal meshes to poseable meshes)
bool ASLVizManager::ConvertWorldToVisualizationMode()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (EpisodeManager->IsWorldConverted())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s world is already converted to viz mode.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return true;
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

	EpisodeManager->ConvertWorld();
	return EpisodeManager->IsWorldConverted();
}

// Check if world is set for episode replay
bool ASLVizManager::IsWorldConvertedToVisualizationMode() const
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	return EpisodeManager->IsWorldConverted();
}

// Cache the episode data
bool ASLVizManager::CacheEpisodeData(const FString& Id, const TArray<TPair<float, TMap<FString, FTransform>>>& InMongoEpisodeData)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	if (InMongoEpisodeData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s the episode data is empty.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	if (IsEpisodeCached(Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s the episode data is already cached.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return true;
	}

	// Create and reserve episode data with the array size
	FSLVizEpisodeData VizEpisodeData(InMongoEpisodeData.Num());
	VizEpisodeData.Id = Id;
	if (FSLVizEpisodeUtils::BuildEpisodeData(IndividualManager, InMongoEpisodeData, VizEpisodeData))
	{
		CachedEpisodeData.Add(Id, VizEpisodeData);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s could not generate episode format.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
}

// Load cached episode data
bool ASLVizManager::LoadCachedEpisodeData(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	if (!EpisodeManager->IsWorldConverted())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s cannot load episode data because the world is not set as visual only.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	if (!IsEpisodeCached(Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s the episode (%s) data is not cached.."), *FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}
	
	EpisodeManager->LoadEpisode(CachedEpisodeData[Id]);
	return true;
}

// Replay cached episode 
bool ASLVizManager::ReplayCachedEpisode(const FString& Id, const FSLVizEpisodePlayParams& Params)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	if (!IsEpisodeCached(Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s episode (%s) is not cached.."), *FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}
	if (!EpisodeManager->GetEpisodeId().Equals(Id))
	{
		EpisodeManager->LoadEpisode(CachedEpisodeData[Id]);
	}

	return EpisodeManager->Play(Params);
}

// Goto cached episode frame
bool ASLVizManager::GotoCachedEpisodeFrame(const FString& Id, float Ts)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	if (!IsEpisodeCached(Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s episode (%s) is not cached.."), *FString(__FUNCTION__), __LINE__, *GetName(), *Id);
		return false;
	}
	if (!EpisodeManager->GetEpisodeId().Equals(Id))
	{
		EpisodeManager->LoadEpisode(CachedEpisodeData[Id]);
	}

	return EpisodeManager->GotoFrame(Ts);
}

// Change the data into an episode format and load it to the episode replay manager
void ASLVizManager::LoadEpisodeData(const TArray<TPair<float, TMap<FString, FTransform>>>& InMongoEpisodeData)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!EpisodeManager->IsWorldConverted())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s cannot load episode data because the world is not set as visual only.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (InMongoEpisodeData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s the episode data is empty.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}


	// Create and reserve episode data with the array size
	FSLVizEpisodeData VizEpisodeData(InMongoEpisodeData.Num());
	if (FSLVizEpisodeUtils::BuildEpisodeData(IndividualManager, InMongoEpisodeData, VizEpisodeData))
	{
		EpisodeManager->LoadEpisode(VizEpisodeData);
	}
}

// Check if any episode is loaded (return the name of the episode)
bool ASLVizManager::IsEpisodeLoaded() const
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	return EpisodeManager->IsEpisodeLoaded();
}

// Go to the frame at the given timestamp
bool ASLVizManager::GotoEpisodeFrame(float Ts)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	return EpisodeManager->GotoFrame(Ts);
}

// Replay the whole loaded episode
bool ASLVizManager::PlayEpisode(FSLVizEpisodePlayParams PlayParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	EpisodeManager->SetReplayParams(PlayParams.bLoop, PlayParams.UpdateRate, PlayParams.StepSize);
	if (PlayParams.StartTime < 0.f && PlayParams.EndTime < 0.f)
	{
		return EpisodeManager->PlayEpisode();
	}
	else
	{
		return EpisodeManager->PlayTimeline(PlayParams.StartTime, PlayParams.EndTime);
	}
}

// Replay the selected timeline in the episode
bool ASLVizManager::PlayEpisodeTimeline(float StartTime, float EndTime, FSLVizEpisodePlayParams PlayParams)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}
	EpisodeManager->SetReplayParams(PlayParams.bLoop, PlayParams.UpdateRate, PlayParams.StepSize);
	return EpisodeManager->PlayTimeline(StartTime, EndTime);
}

// Pause/unpause the replay (if active)
void ASLVizManager::PauseReplay(bool bPause)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	EpisodeManager->SetPauseReplay(bPause);
}

// Stop replay (if active, and goto frame 0)
void ASLVizManager::StopReplay()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	EpisodeManager->StopReplay();
}

// Move the view to a given position
void ASLVizManager::SetCameraView(const FTransform& Pose)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	CameraDirector->MoveCameraTo(Pose);
}

// Move the camera view to the pose of the given individual
void ASLVizManager::SetCameraView(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (auto Individual = IndividualManager->GetIndividual(Id))
	{
		CameraDirector->MoveCameraTo(Individual->GetParentActor()->GetActorTransform());
	}
}

// Attach the view to an individual
void ASLVizManager::AttachCameraViewTo(const FString& Id)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (auto Individual = IndividualManager->GetIndividual(Id))
	{
		CameraDirector->AttachCameraTo(Individual->GetParentActor());
	}
}

// Make sure the camera view is detached
void ASLVizManager::DetachCameraView()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, call init first.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	CameraDirector->DetachCamera();
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
#if WITH_EDITOR
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
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
#if WITH_EDITOR
	HighlightManager->SetActorLabel(TEXT("SL_VizHighlightManager"));
#endif // WITH_EDITOR
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
#if WITH_EDITOR
	MarkerManager->SetActorLabel(TEXT("SL_VizMarkerManager"));
#endif // WITH_EDITOR
	return true;
}

// Get the vizualization episode replay manager from the world (or spawn a new one)
bool ASLVizManager::SetEpisodeManager()
{
	if (EpisodeManager && EpisodeManager->IsValidLowLevel() && !EpisodeManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizEpisodeManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			EpisodeManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_EpisodeManager");
	EpisodeManager = GetWorld()->SpawnActor<ASLVizEpisodeManager>(SpawnParams);
#if WITH_EDITOR
	EpisodeManager->SetActorLabel(TEXT("SL_EpisodeManager"));
#endif // WITH_EDITOR
	return true;
}

// Get the vizualization camera director from the world (or spawn a new one)
bool ASLVizManager::SetCameraDirector()
{
	if (CameraDirector && CameraDirector->IsValidLowLevel() && !CameraDirector->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizCameraDirector>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			CameraDirector = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizCameraDirector");
	CameraDirector = GetWorld()->SpawnActor<ASLVizCameraDirector>(SpawnParams);
#if WITH_EDITOR
	CameraDirector->SetActorLabel(TEXT("SL_VizCameraDirector"));
#endif // WITH_EDITOR
	return true;
}
