// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/TriggerBox.h"
#include "SLContactTriggerBox.generated.h"

/**
 *  Trigger box sending contact information to the semantic events exporter
 */
UCLASS()
class SEMLOG_API ASLContactTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

	// Constructor
	ASLContactTriggerBox();
	
	// Destructor
	~ASLContactTriggerBox();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Callback on begin overlap
	UFUNCTION()
	void BeginSemanticContact(AActor* Self, AActor* OtherActor);

	// Callback on end overlap
	UFUNCTION()
	void EndSemanticContact(AActor* Self, AActor* OtherActor);
	
	// Create raster
	void CreateRaster();

	// Check particle cound on raster
	void CheckParticleCount();

	// Static mesh actor parent for the trigger
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	AStaticMeshActor* Parent;

	// Create raster for particle collisions
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bCreateRaster;

	// Number of rows in the raster
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint32 RasterNrRows;

	// Number of columns in the raster
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint32 RasterNrColumns;

	// Particle collision update rate
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float RasterUpdateRate;

	// Raster visibility
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bRasterHiddenInGame;

	// Array of trigger boxes from the raster
	TArray<UBoxComponent*> RasterTriggerBoxes;

	// Timer handle to checks for raster components collisions
	FTimerHandle TimerHandle;

	// Pointer to the semantic events exporter
	class FSLEventsExporter* SemEventsExporter;
};
