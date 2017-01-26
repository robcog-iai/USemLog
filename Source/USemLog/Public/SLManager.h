// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/Actor.h"
#include "Private/SLRawDataExporter.h"
#include "Private/SLMapExporter.h"
#include "Private/SLEventsExporter.h"
#include "SLManager.generated.h"

UENUM()
enum class ESLManagerState : uint8
{
	UnInit			UMETA(DisplayName = "UnInit"),
	PreInit         UMETA(DisplayName = "PreInit"),
	Init			UMETA(DisplayName = "Init"),
	Active			UMETA(DisplayName = "Active"),
	Paused			UMETA(DisplayName = "Paused"),
	Stopped			UMETA(DisplayName = "Stopped"),
	Cancelled		UMETA(DisplayName = "Stopped"),
};

UCLASS()
class USEMLOG_API ASLManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLManager();
	
	// Actor initialization, log items init
	virtual void PreInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called when the game is terminated
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Pre init exporters, setup folders, read previous values
	bool PreInit();

	// Init exporters, write initial states
	bool Init();

	// Start logging by enabling tick and listening to events
	bool Start();
	
	// Pause logging by disabling tick and listening to events
	bool Pause();

	// Stop the loggers, save states to file
	bool Stop();
	
	// TODO Cancelled() - RM files (check if the map was created now, then remove it, if not only the episode)

	// Check if state is uninit
	bool IsUnInit() { return ManagerState == ESLManagerState::UnInit; };

	// Check if state is pre init
	bool IsPreInit() { return ManagerState == ESLManagerState::PreInit; };

	// Check if state is init
	bool IsInit() { return ManagerState == ESLManagerState::Init; };

	// Check if state is active
	bool IsActive() { return ManagerState == ESLManagerState::Active; };

	// Check if state is active
	bool IsPaused() { return ManagerState == ESLManagerState::Paused; };

	// Check if state is stopped
	bool IsStopped() { return ManagerState == ESLManagerState::Stopped; };

	// Get semantic events exporter
	class FSLEventsExporter* GetEventsExporter() { return SemEventsExporter; };

	// Semantic logger manager state
	ESLManagerState ManagerState;

private:
	// Create directory path for logging
	void CreateDirectoryPath(FString Path);

	// Set items to be loggeed (from tags)
	void InitLogItems();

	// Check if new unique names should be generated (previous ones are not stored)
	bool ShouldGenerateNewUniqueNames(const FString Path);

	// Set unique names of items
	void GenerateNewUniqueNames();

	// Write generated unique names to file
	void StoreNewUniqueNames(const FString Path);

	// Cancel logging (e.g if objects are out of sync)
	void CancelLogging();

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartLoggingAtLoadTime;

	// Directory to save the logs
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString LogRootDirectoryName;

	// Log raw data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogRawData;

	// Distance threshold for raw data logging (cm)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float DistanceThreshold;

	// Log semantic map (if not already logged)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogSemanticMap;

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogSemanticEvents;

	// Log landscape components (e.g. roads, foliage)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogLandscapeComponents;

	// Log foliage classes
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<FString> LogFoliageClasses;


	// Map of actors to be logged to the semlog key-value pair info
	TMap<AActor*, TArray<TPair<FString, FString>>> ActorToSemLogInfo;

	// Map of actors to be logged to their unique name
	TMap<AActor*, FString> ActorToUniqueName;

	// Map of the foliage class name to the component
	TMap<FString, UInstancedStaticMeshComponent*> FoliageClassNameToComponent;

	// Map of foliage class to array of bodies with unique names 
	TMap<UInstancedStaticMeshComponent*, TArray<TPair<FBodyInstance*, FString>>> FoliageComponentToUniqueNameArray;

	// Road components name to component //TODO only one road, add to sherpa branch
	TMap<FString, USceneComponent*> RoadCompNameToComponent;

	// Road components name to unique names //TODO only one road, add to sherpa branch
	TMap<FString, FString> RoadComponentNameToUniqueName;

	// Road unique name //TODO only one road, add to sherpa branch
	FString RoadUniqueName;

	// Raw data exporter
	class FSLRawDataExporter* RawDataExporter;

	// Semantic map exporter
	class FSLMapExporter* SemMapExporter;

	// Semantic events exporter
	class FSLEventsExporter* SemEventsExporter;

	// Level path
	FString LevelPath;

	// Episode path
	FString EpisodePath;

	// Raw data path
	FString RawDataPath;

	// Episode unique tag
	FString EpisodeUniqueTag;
};
