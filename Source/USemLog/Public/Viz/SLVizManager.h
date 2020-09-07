// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLVizManager.generated.h"

// Forward declarations
class ASLVizMarkerManager;
class ASLVizHighlightMarkerManager;
class ASLVizWorldManager;
class ASLIndividualManager;
class USLVizHighlightMarker;

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

	//// Dtor
	//~ASLVizManager();

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

	// Highlight the individual (returns false if the individual is not found or is not of visual type)
	bool HighlightIndividual(const FString& Id);

	// Remove highlight from individual (returns false if the individual not found or it is not highlighted)
	bool RemoveIndividualHighlight(const FString& Id);

	// Remove all individual highlights
	bool RemoveAllIndividualHighlights();

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

	// Individual id to query
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	FString IndividualIdValueHack = TEXT("DefaultIndividualId");
	
	// Highlight individual button hack
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bHighlightButtonHack = false;

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
