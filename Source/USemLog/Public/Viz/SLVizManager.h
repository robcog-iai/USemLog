// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/SLVizHighlightMarker.h"
#include "SLVizManager.generated.h"

// Forward declarations
class USLVizMarker;
class ASLVizMarkerManager;
class USLVizHighlightMarker;
class ASLVizHighlightMarkerManager;
class ASLVizWorldManager;
class ASLIndividualManager;
class USLVizHighlightMarker;

/**
 * Highlight individuals test hack struct
 */
USTRUCT()
struct FSLVizHighlightIndividualCmdHack
{
	GENERATED_BODY();

	// Individual id to query
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	FString IndividualId = TEXT("DefaultIndividualId");

	// Highlight visual
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FSLVizHighlightMarkerVisualParams VisualParams;
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


	/* Markers */
	// Create marker with the given id
	bool CreateMarker(const FString& Id);

	// Update the visual values of the marker
	bool UpdateMarker(const FString& Id);

	// Remove marker with the given id
	bool RemoveMarker(const FString& Id);

	// Remove all markers
	void RemoveAllMarkers();


	/* Highlights */
	// Highlight the individual (returns false if the individual is not found or is not of visual type)
	bool HighlightIndividual(const FString& Id, const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Change the visual values of the highligted individual
	bool UpdateIndividualHighlight(const FString& Id, const FSLVizHighlightMarkerVisualParams& VisualParams);

	// Remove highlight from individual (returns false if the individual not found or it is not highlighted)
	bool RemoveIndividualHighlight(const FString& Id);

	// Remove all individual highlights
	void RemoveAllIndividualHighlights();

private:
	// Get the vizualization marker manager from the world (or spawn a new one)
	bool SetVizMarkerManager();

	// Get the vizualization highlight marker manager from the world (or spawn a new one)
	bool SetVizHighlightMarkerManager();

	// Get the vizualization world manager from the world (or spawn a new one)
	bool SetVizWorldManager();

	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();
	
private:
	// True if the manager is initialized
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	bool bIsInit;

	// Keep track of the markers
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<FString, USLVizMarker*> Markers;

	// Keep track of the highlighted individuals
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TMap<FString, USLVizHighlightMarker*> HighlightedIndividuals;

	// Keeps track of all the drawn markers in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizMarkerManager* VizMarkerManager;

	// Keeps track of all the highlight markers in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizHighlightMarkerManager* VizHighlightMarkerManager;

	// Keeps track of the episode replay visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizWorldManager* VizWorldManager;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;



	/* Editor button hacks */
	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bInitButtonHack = false;

	// Highlight commands array
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	TArray<FSLVizHighlightIndividualCmdHack> HighlightValuesHack;
	
	// Highlight individual button hack
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bHighlightButtonHack = false;

	// Update higlight visual values
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bUpdateHighlightButtonHack = false;

	// Remove highlight individual button hack
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bRemoveHighlightButtonHack = false;

	// Remove all individual highlights
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bRemoveAllHighlightsButtonHack = false;

	// Clear any created markers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bResetButtonHack = false;
};
