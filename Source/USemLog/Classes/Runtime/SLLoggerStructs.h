// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLLoggerStructs.generated.h"

/* Save location info */
USTRUCT()
struct FSLLoggerLocationParams
{
	GENERATED_BODY();

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

	// Overwrite any exiting data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bOverwrite = false;
};

/* DB Server location info */
USTRUCT()
struct FSLLoggerDBServerParams
{
	GENERATED_BODY();

	// Database Server Ip
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString Ip = TEXT("127.0.0.1");

	// Database server port num
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (ClampMin = 0, ClampMax = 65535))
	uint16 Port = 27017;
};

/* Logger start options */
UENUM()
enum class ESLLoggerStartTime : uint8
{
	AtBeginPlay         UMETA(DisplayName = "AtBeginPlay"),
	AtNextTick			UMETA(DisplayName = "AtNextTick"),
	AfterDelay			UMETA(DisplayName = "AfterDelay"),
	FromUserInput       UMETA(DisplayName = "FromUserInput"),
};

/* Start logger info */
USTRUCT()
struct FSLLoggerStartParams
{
	GENERATED_BODY();

	// Reset start time to 0 when starting to log
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bResetStartTime = false;

	// Logger start time
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLLoggerStartTime StartTime = ESLLoggerStartTime::AtBeginPlay;

	// Start after a given delay (if ESLLoggerStartTime::AfterDelay is selected)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger"/*, meta = (editcondition = todo 4.22+)*/)
	float StartDelay = 0.5f;

	// Action name for starting from user input (if ESLLoggerStartTime::FromUserInput is selected)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger"/*, meta = (editcondition = todo 4.22+)*/)
	FName UserInputActionName = TEXT("SLTrigger");
};

/* Holds the data needed to setup the world state logger */
USTRUCT()
struct FSLWorldStateLoggerParams
{
	GENERATED_BODY();

	// Update rate of the logger
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate = 0.f;

	// Min difference between poses (FTransform) in order for the individual to be logged
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float PoseTolerance = 0.5f;

	//// Min linear distance to log an individual (cm)
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	//float MinLinearDistance = 0.5f;

	//// Min angular distance in order to log an individual (rad)
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	//float MinAngularDistance = 0.1;

	//// If available, log gaze data
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	//bool bLogGazeData = true;
};