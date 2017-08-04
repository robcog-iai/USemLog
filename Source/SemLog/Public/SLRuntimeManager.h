// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLRawDataLogger.h"
#include "SLEventDataLogger.h"
#include "SLRuntimeManager.generated.h"

UCLASS()
class SEMLOG_API ASLRuntimeManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLRuntimeManager();

protected:
	// Make sure the manager is started before event publishers call BeginPlay
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Init loggers
	void Init();

	// Start loggers
	void Start();

	// Finish loggers
	void Finish();

	// Check if init
	bool IsInit() const { return bIsInit; };

	// Check if started
	bool IsStarted() const { return bIsStarted; };

	// Check if finished
	bool IsFinished() const { return bIsFinished; };

	// Get raw data logger
	USLRawDataLogger* GetRawDataLogger() { return RawDataLogger; };

	// Get event data logger
	USLEventDataLogger* GetEventDataLogger() { return EventDataLogger; };

	// Add finished event
	bool AddFinishedEvent(TSharedPtr<FOwlNode> Event);

	// Start an event
	bool StartEvent(TSharedPtr<FOwlNode> Event);

	// Finish an event
	bool FinishEvent(TSharedPtr<FOwlNode> Event);

	// Add metadata property
	bool AddMetadataProperty(TSharedPtr<FOwlTriple> Property);

	// Add new item to be logged during runtime
	void AddNewEntity(AActor* Actor);

	// Stop logging the entity
	void RemoveEntity(AActor* Actor);

	// Get episode ID
	FString GetEpisodeId() const { return EpisodeId; };

private:
	// Episode Id (be default will be auto generated)
	UPROPERTY(EditAnywhere, Category = "SL")
	FString EpisodeId;

	// Log directory
	UPROPERTY(EditAnywhere, Category = "SL")
	FString LogDirectory;

	// Start at load time
	UPROPERTY(EditAnywhere, Category = "SL")
	uint8 bStartAtLoadTime : 1;

	// Logger init
	uint8 bIsInit : 1;

	// Logger started
	uint8 bIsStarted : 1;

	// Logger finished
	uint8 bIsFinished : 1;

	// Log raw data
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger")
	bool bLogRawData;

	// Distance (cm) threshold for logging raw data
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float RawDataDistanceThreshold;

	// Update rate in seconds (if 0, or smaller than the Tick's DeltaTime, it will update every tick)
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float RawDataUpdateRate;

	// Write data to file
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	uint8 bWriteRawDataToFile : 1;

	// Broadcast data
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	uint8 bBroadcastRawData : 1;

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "SL|Event Data Logger")
	uint8 bLogEventData : 1;

	// Write data to file
	UPROPERTY(EditAnywhere, Category = "SL|Event Data Logger", meta = (editcondition = "bLogEventData"))
	uint8 bWriteEventDataToFile : 1;

	// Write event data as timelines as well
	UPROPERTY(EditAnywhere, Category = "SL|Event Data Logger", meta = (editcondition = "bLogEventData"))
	uint8 bWriteEventTimelines : 1;

	// Broadcast data
	UPROPERTY(EditAnywhere, Category = "SL|Event Data Logger", meta = (editcondition = "bLogEventData"))
	uint8 bBroadcastEventData : 1;

	// Raw data logger
	UPROPERTY()
	USLRawDataLogger* RawDataLogger;

	// Event data logger
	UPROPERTY()
	USLEventDataLogger* EventDataLogger;

	// Amount of time passed since last update (used for the raw data update rate)
	float TimePassedSinceLastUpdate;
};
