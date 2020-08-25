// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLWorldStateLogger.generated.h"

/* Holds the data needed to setup the world state logger */
USTRUCT()
struct FSLWorldStateLoggerParams
{
	GENERATED_BODY();

	// Update rate of the logger
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate = 0.f;

	// Min squared linear distance to log an individual
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float MinLinearDistanceSquared = 0.25f;

	// Min angular distance in order to log an individual
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float MinAngularDistance = 0.1; // rad

	// Set to true in order to edit the episode id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomTaskId = false;

	// Unique id of the task
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseCustomTaskId"))
	FString TaskId = TEXT("DefaultTaskId");

	// Set to true in order to edit the episode id
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseCustomEpisodeId = false;

	// Unique id of the episode
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseCustomEpisodeId"))
	FString EpisodeId = TEXT("DefaultEpisodeId");

	// Database Server Ip
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString ServerIp = TEXT("127.0.0.1");

	// Database server port num
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (ClampMin = 0, ClampMax = 65535))
	uint16 ServerPort = 27017;

	// Overwrite any exiting data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bOverwrite = false;

	// If true the logger will start on its own (instead of being started by the manager)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartIndependently = false;

	// Reset start time to 0 when starting to log
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartIndependently"))
	bool bResetStartTime = false;

	// Start logger at begin play
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartIndependently"))
	bool bStartAtBeginPlay = true;

	// Start logger at first tick
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartIndependently"))
	bool bStartAtFirstTick = false;

	// Start after a given delay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartIndependently"))
	bool bStartWithDelay = false;

	// Start after a given delay
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartWithDelay"))
	float StartDelay = 0.5f;

	// Start from external user input
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartIndependently"))
	bool bStartFromUserInput = false;

	// Action name for starting from user input
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bStartFromUserInput"))
	FName UserInputActionName = TEXT("SLTrigger");
};

/**
 * Subsymbolic data logger
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL World State Logger")
class USEMLOG_API ASLWorldStateLogger : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLWorldStateLogger();

	// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
	virtual void PostLoad() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Init using external parameters
	void Init(const FSLWorldStateLoggerParams& InParams);

	// Init using internal parameters
	void Init();

	// Start logging
	void Start();

	// Finish logging
	void Finish();

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Logger parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FSLWorldStateLoggerParams Params;

private:
	// True when ready to log
	bool bIsInit;

	// True when active
	bool bIsStarted;

	// True when done logging
	bool bIsFinished;
};
