// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLVizSemMapManager.generated.h"

// Forward declarations
class ASLIndividualManager;
class ASLVizManager;

/**
 * 
 */
UCLASS()
class ASLVizSemMapManager : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLVizSemMapManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Load required managers, setup world
	void Init();

	// Check if the manager is initialized
	bool IsInit() const { return bIsInit; };

	// Clear changes to the world
	void Reset();

	// Hide all individuals in the world
	void HideAllIndividuals();

	// Hide all individuals in the world
	void ShowAllIndividuals();
	
	// Hide selected individuals
	void HideIndividuals(const TArray<FString>& Ids, bool bIterate);

	// Show selected individuals
	void ShowIndividuals(const TArray<FString>& Ids, bool bIterate);

protected:
	/* Managers */
	// Get the individual manager from the world (or spawn a new one)
	bool SetIndividualManager();

	// Get the viz manager from the world (or spawn a new one)
	bool SetVizManager();

protected:
	// True when successfully initialized
	bool bIsInit;

	// True if a task is still executing (timer active)
	bool bExecutingTask;

	// Keeps access to all the individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Marker visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizManager* VizManager;

private:
	// Execute tasks iteratively
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bIterate = false;

	// Interval on which to execute the task
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float bIterateInterval = 0.05f;

	// Iterate execution timer handle
	FTimerHandle IterateTimerHandle;
};
