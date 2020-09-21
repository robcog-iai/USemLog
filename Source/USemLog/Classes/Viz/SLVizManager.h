// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/SLVizStructs.h"
#include "SLVizManager.generated.h"

// Forward declarations
class ASLVizHighlightManager;
class ASLVizMarkerManager;
class ASLVizEpisodeReplayManager;
class ASLIndividualManager;
class USLVizBaseMarker;
class UMeshComponent;

/**
 * Highlight individuals test hack struct
 */
USTRUCT()
struct FSLVizHighlightTestStruct
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString IndividualId = TEXT("DefaultIndividualId");

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;
};


/**
 * Marker test hack struct
 */
USTRUCT()
struct FSLVizPrimitiveMarkerTestStruct
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<FTransform> Poses;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString MarkerId = TEXT("DefaultMarkerId");

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float Size = .1f;
		
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;
};


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

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Load required managers
	bool Init();

	// Check if the manager is initialized
	bool IsInit() const { return bIsInit; };

	// Clear any created markers / viz components
	void Reset();


	/* Highlights */
	// Highlight the individual (returns false if the individual is not found or is not of visual type)
	bool HighlightIndividual(const FString& Id,
		const FLinearColor& Color = FLinearColor::Green,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Change the visual values of the highligted individual
	bool UpdateIndividualHighlight(const FString& Id,
		const FLinearColor& Color,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Remove highlight from individual (returns false if the individual not found or it is not highlighted)
	bool RemoveIndividualHighlight(const FString& Id);

	// Remove all individual highlights
	void RemoveAllIndividualHighlights();


	/* Markers */
	// Create a primitive marker
	bool CreatePrimitiveMarker(const FString& MarkerId,
		const TArray<FTransform>& Poses,
		ESLVizPrimitiveMarkerType PrimitiveType,
		float Size,
		const FLinearColor& Color = FLinearColor::Green,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a marker by cloning the visual of the given individual (use original materials)
	bool CreateStaticMeshMarker(const FString& MarkerId, const TArray<FTransform>& Poses,
		const FString& IndividualId);

	// Create a marker by cloning the visual of the given individual
	bool CreateStaticMeshMarker(const FString& MarkerId,
		const TArray<FTransform>& Poses,
		const FString& IndividualId,
		const FLinearColor& Color,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a marker by cloning the visual of the given skeletal individual (use original materials)
	bool CreateSkeletalMeshMarker(const FString& MarkerId,
		const TArray<FTransform>& Poses,
		const TArray<TMap<int32, FTransform>>& BonePoses,
		const FString& IndividualId);

	// Create a marker by cloning the visual of the given skeletal individual
	bool CreateSkeletalMeshMarker(const FString& MarkerId,
		const TArray<FTransform>& Poses,
		const TArray<TMap<int32, FTransform>>& BonePoses,
		const FString& IndividualId,
		const FLinearColor& Color,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a marker by cloning the visual of the given individual (use original materials)
	bool CreateBoneMeshMarker(const FString& MarkerId,
		const TArray<FTransform>& Poses,
		const FString& IndividualId);

	// Create a marker by cloning the visual of the given individual
	bool CreateBoneMeshMarker(const FString& MarkerId,
		const TArray<FTransform>& Poses,
		const FString& IndividualId,
		const FLinearColor& Color,
		ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Remove marker with the given id
	bool RemoveMarker(const FString& Id);

	// Remove all markers
	void RemoveAllMarkers();


	/* Episode */
	// Setup the world for episode replay (remove physics, pause simulation, change skeletal meshes to poseable meshes)
	bool SetupWorldForEpisodeReplay();

	// Check if world is set for episode replay
	bool IsWorldSetForEpisodeReplay() const;

	// Change the data into an episode format and load it to the episode replay manager
	void LoadEpisodeData(const TArray<TPair<float, TMap<FString, FTransform>>>& InCompactEpisodeData);

	// Check if any episode is loaded (return the name of the episode)
	bool IsEpisodeLoaded() const;

	// Go to the frame at the given timestamp
	bool GotoEpisodeFrame(float Ts);

	// Replay the whole loaded episode
	bool PlayEpisode(bool bLoop = false, float UpdateRate = -1.f, int32 StepSize = 1);

	// Replay the whole loaded episode
	bool PlayEpisodeTimeline(float StartTime, float EndTime, bool bLoop = false, float UpdateRate = -1.f, int32 StepSize = 1);

	// Pause/unpause the replay (if active)
	void PauseReplay(bool bPause);

	// Stop replay (if active, and goto frame 0)
	void StopReplay();

private:
	/* Managers */
	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Get the vizualization highlight manager from the world (or spawn a new one)
	bool SetVizHighlightManager();

	// Get the vizualization marker manager from the world (or spawn a new one)
	bool SetVizMarkerManager();

	// Get the vizualization world manager from the world (or spawn a new one)
	bool SetEpisodeReplayManager();
	
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
	// Keeps track of all the drawn markers in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizMarkerManager* MarkerManager;

	// Keeps track of all the highlighted meshes in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizHighlightManager* HighlightManager;

	// Keeps track of the episode replay visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizEpisodeReplayManager* EpisodeReplayManager;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;




	/* Editor button hacks */
	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteInitButtonHack = false;

	// Execute marker commands
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteButtonHack = false;

	// Update markers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteUpdateButtonHack = false;

	// Remove highlight individual button hack
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteRemoveButtonHack = false;

	// Remove all individual highlights
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteRemoveAllButtonHack = false;

	// Clear any created markers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteResetButtonHack = false;

	// Highlight test
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	TArray<FSLVizHighlightTestStruct> HighlightTestValuesHack;

	// Primitive marker tests
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	TArray<FSLVizPrimitiveMarkerTestStruct> PrimitiveMarkerTestHack;

	// Ids to remove can be highlights or markers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	TArray<FString> RemoveTestHack;

	// Setup world to be ready for episode replay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bExecuteReplaySetupButtonHack = false;
};
