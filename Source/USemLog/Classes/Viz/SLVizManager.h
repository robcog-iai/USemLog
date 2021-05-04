// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/SLVizStructs.h"
#include "Viz/SLVizEpisodeManager.h"
#include "SLVizManager.generated.h"

// Forward declarations
class ASLVizHighlightManager;
class ASLVizMarkerManager;
//class ASLVizEpisodeManager;
class ASLIndividualManager;
class ASLVizCameraDirector;
class USLVizBaseMarker;
class UMeshComponent;

/*
*
*/
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Manager")
class USEMLOG_API ASLVizManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLVizManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Load required managers
	void Init();

	// Check if the manager is initialized
	bool IsInit() const { return bIsInit; };

	// Clear any created markers / viz components
	void Reset();

	// Get the individual manager
	ASLIndividualManager* GetIndividualManager() const { return IndividualManager; };

	/* Highlights */
	// Highlight the individual (returns false if the individual is not found or is not of visual type)
	bool HighlightIndividual(const FString& Id,
		const FLinearColor& Color = FLinearColor::Green, ESLVizMaterialType MaterialType = ESLVizMaterialType::Translucent);

	// Change the visual values of the highligted individual
	bool UpdateIndividualHighlight(const FString& Id,
		const FLinearColor& Color, ESLVizMaterialType MaterialType = ESLVizMaterialType::Translucent);

	// Remove highlight from individual (returns false if the individual not found or it is not highlighted)
	bool RemoveIndividualHighlight(const FString& Id);

	// Remove all individual highlights
	void RemoveAllIndividualHighlights();

	// Spawn or get manager from the world
	static ASLVizManager* GetExistingOrSpawnNew(UWorld* World);


	/* Primitive markers */
	// Create a primitive marker
	bool CreatePrimitiveMarker(const FString& MarkerId,	const TArray<FTransform>& Poses,
		ESLVizPrimitiveMarkerType PrimitiveType, float Size,
		const FLinearColor& Color = FLinearColor::Green, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a primitive marker timeline
	bool CreatePrimitiveMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
		ESLVizPrimitiveMarkerType PrimitiveType, float Size,
		const FLinearColor& Color, ESLVizMaterialType MaterialType,
		const FSLVizTimelineParams& TimelineParams);


	/* Static mesh markers */
	// Create a marker by cloning the visual of the given individual (use original materials)
	bool CreateStaticMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId);

	// Create a marker by cloning the visual of the given individual
	bool CreateStaticMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a timeline marker by cloning the visual of the given individual (use original materials)
	bool CreateStaticMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId,
		const FSLVizTimelineParams& TimelineParams);

	// Create a timeline marker by cloning the visual of the given individual
	bool CreateStaticMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId, const FLinearColor& Color, ESLVizMaterialType MaterialType,
		const FSLVizTimelineParams& TimelineParams);


	/* Skeletal mesh markers */
	// Create a marker by cloning the visual of the given skeletal individual (use original materials)
	bool CreateSkeletalMeshMarker(const FString& MarkerId,
		const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		const FString& IndividualId);

	// Create a marker by cloning the visual of the given skeletal individual
	bool CreateSkeletalMeshMarker(const FString& MarkerId, 
		const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		const FString& IndividualId,
		const FLinearColor& Color, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a timeline by cloning the visual of the given skeletal individual (use original materials)
	bool CreateSkeletalMeshMarkerTimeline(const FString& MarkerId,
		const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		const FString& IndividualId,
		const FSLVizTimelineParams& TimelineParams);

	// Create a timeline by cloning the visual of the given skeletal individual
	bool CreateSkeletalMeshMarkerTimeline(const FString& MarkerId,
		const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		const FString& IndividualId,
		const FLinearColor& Color, ESLVizMaterialType MaterialType,
		const FSLVizTimelineParams& TimelineParams);

	/* Skeletal bone mesh markers */
	// Create a marker by cloning the visual of the given individual (use original materials)
	bool CreateBoneMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId);

	// Create a marker by cloning the visual of the given individual
	bool CreateBoneMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId,
		const FLinearColor& Color, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a timeline by cloning the visual (bone only) of the given skeletal individual (use original materials)
	bool CreateBoneMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId,
		const FSLVizTimelineParams& TimelineParams);

	// Create a timeline by cloning the visual (bone only) of the given skeletal individual
	bool CreateBoneMeshMarkerTimeline(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId,
		const FLinearColor& Color, ESLVizMaterialType MaterialType,
		const FSLVizTimelineParams& TimelineParams);

	// Remove marker with the given id
	bool RemoveMarker(const FString& Id);

	// Remove all markers
	void RemoveAllMarkers();

	/* Episode */
	// Setup the world for episode replay (remove physics, pause simulation, change skeletal meshes to poseable meshes)
	bool ConvertWorldToVisualizationMode();

	// Check if world is set for episode replay
	bool IsWorldConvertedToVisualizationMode() const;

	// Cache the mongo data into an episode format
	bool CacheEpisodeData(const FString& Id, const TArray<TPair<float, TMap<FString, FTransform>>>& InMongoEpisodeData);

	// Check if the episode is already cached
	bool IsEpisodeCached(const FString& Id) const { return CachedEpisodeData.Contains(Id); };

	// Load cached episode data
	bool LoadCachedEpisodeData(const FString& Id);

	// Replay cached episode 
	bool ReplayCachedEpisode(const FString& Id, const FSLVizEpisodePlayParams& Params = FSLVizEpisodePlayParams());

	// Goto cached episode frame
	bool GotoCachedEpisodeFrame(const FString& Id, float Ts);

	// Change the data into an episode format and load it to the episode replay manager
	void LoadEpisodeData(const TArray<TPair<float, TMap<FString, FTransform>>>& InCompactEpisodeData);

	// Check if any episode is loaded (return the name of the episode)
	bool IsEpisodeLoaded() const;

	// Go to the frame at the given timestamp
	bool GotoEpisodeFrame(float Ts);

	// Replay the whole loaded episode
	bool PlayEpisode(FSLVizEpisodePlayParams PlayParams = FSLVizEpisodePlayParams());

	// Replay the whole loaded episode
	bool PlayEpisodeTimeline(float StartTime, float EndTime, FSLVizEpisodePlayParams PlayParams = FSLVizEpisodePlayParams());

	// Pause/unpause the replay (if active)
	void PauseReplay(bool bPause);

	// Stop replay (if active, and goto frame 0)
	void StopReplay();


	/* View */
	// Move the view to a given position
	void SetCameraView(const FTransform& Pose);

	// Move the camera view to the pose of the given individual
	void SetCameraView(const FString& Id);

	// Attach the view to an individual
	void AttachCameraViewTo(const FString& Id);

	// Make sure the camera view is detached
	void DetachCameraView();

private:
	/* Managers */
	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Get the vizualization highlight manager from the world (or spawn a new one)
	bool SetVizHighlightManager();

	// Get the vizualization marker manager from the world (or spawn a new one)
	bool SetVizMarkerManager();

	// Get the vizualization world manager from the world (or spawn a new one)
	bool SetEpisodeManager();

	// Get the vizualization camera director from the world (or spawn a new one)
	bool SetCameraDirector();
	
private:
	// True if the manager is initialized
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	bool bIsInit;

	// Keep track of the markers in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<FString, USLVizBaseMarker*> Markers;

	// Keep track of the highlighted individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<FString, FSLVizIndividualHighlightData> HighlightedIndividuals;


	/* Managers */
	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Keeps track of all the drawn markers in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizMarkerManager* MarkerManager;

	// Keeps track of all the highlighted meshes in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizHighlightManager* HighlightManager;

	// Keeps track of the episode replay visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizEpisodeManager* EpisodeManager;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizCameraDirector* CameraDirector;


	/* Cached data */
	// Episode id to viz episode data
	TMap<FString, FSLVizEpisodeData> CachedEpisodeData;
};
