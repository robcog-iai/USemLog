// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/Actor.h"
#include "Private/SLRawDataExporter.h"
#include "Private/SLMapExporter.h"
#include "Private/SLEventsExporter.h"
#include "Private/SLOwlUtils.h"
#include "SLManager.generated.h"

UENUM()
enum class ESLManagerState : uint8
{
	UnInit			UMETA(DisplayName = "UnInit"),
	PreInit         UMETA(DisplayName = "PreInit"),
	Init			UMETA(DisplayName = "Init"),
	Active			UMETA(DisplayName = "Active"),
	Paused			UMETA(DisplayName = "Paused"),
	Finished		UMETA(DisplayName = "Finished"),
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
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool PreInit(const float ExternalTime = 0.f);

	// Init exporters, write initial states
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool Init();

	// Activate logging by enabling tick and listening to events
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool Activate();
	
	// Pause logging by disabling tick and listening to events
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool Pause();

	// Stop the loggers, save states to file
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool Finish();
	
	// Cancel logging, remove generated files
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool Cancel();

	// Check if state is uninit
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsUnInit() { return ManagerState == ESLManagerState::UnInit; };

	// Check if state is pre init
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsPreInit() { return ManagerState == ESLManagerState::PreInit; };

	// Check if state is init
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsInit() { return ManagerState == ESLManagerState::Init; };

	// Check if state is active
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsActive() { return ManagerState == ESLManagerState::Active; };

	// Check if state is active
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsPaused() { return ManagerState == ESLManagerState::Paused; };

	// Check if state is stopped
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsFinished() { return ManagerState == ESLManagerState::Finished; };

	// Check if state is stopped
	UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool IsCancelled() { return ManagerState == ESLManagerState::Cancelled; };

	// Get semantic events exporter
	// TODO rm, or check linking issues
	class FSLEventsExporter* GetEventsExporter() { return SemEventsExporter; };

	// Add finished semantic event
	//UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool AddFinishedSemanticEvent(
		const FString EventNamespace,
		const FString EventName,
		const float StartTime,
		const float EndTime,
		bool bGenerateUniqueIdentifier = true,
		const TArray<FSLOwlTriple>& Properties = TArray<FSLOwlTriple>());

	// Add object individual
	//UFUNCTION(BlueprintCallable, Category = "Semantic Logger")
	bool AddObjectIndividual(
		const FString EventNamespace,
		const FString EventName,
		bool bGenerateUniqueIdentifier = true,
		const TArray<FSLOwlTriple>& Properties = TArray<FSLOwlTriple>());

private:
	// Start logging with delay
	void StartLoggingWithDelay();

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

	// Delay for starting logging at load time (works only if StartLoggingAtLoadTime is true)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float LoadTimeLoggingDelay;

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

	// Semantic logger manager state
	ESLManagerState ManagerState;

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
	FSLRawDataExporter* RawDataExporter;

	// Semantic map exporter
	FSLMapExporter* SemMapExporter;

	// Semantic events exporter
	FSLEventsExporter* SemEventsExporter;

	// Level path
	FString LevelPath;

	// Episode path
	FString EpisodePath;

	// Raw data path
	FString RawDataPath;

	// Episode unique tag
	FString EpisodeUniqueTag;

	// Flag marging first episode
	bool bFirstEpisode;

	// Store external init time (seconds)
	float ExternalInitTime;
};
