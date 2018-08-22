// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLRawDataLogger.h"
#include "SLEventDataLogger.h"
#include "SLContentSingleton.h"
#include "SemanticLogger.generated.h"

/**
 * 
 */
UCLASS()
class USEMLOG_API ASemanticLogger : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASemanticLogger();

	// Destructor
	~ASemanticLogger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 
	virtual void BeginDestroy() override;

	//
	virtual void FinishDestroy() override;

public:
	// Init loggers
	void InitLogging();

	// Start loggers
	void StartLogging();

	// Finish loggers
	void FinishLogging();
	
private:
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

private:
	/* Semantic logger */
	// Episode Id (be default will be auto generated)
	UPROPERTY(EditAnywhere, Category = "SL")
	FString EpisodeId;

	// Log directory (or the database name if saving to mongodb)
	UPROPERTY(EditAnywhere, Category = "SL")
	FString LogDirectory;

	// Start at load time
	UPROPERTY(EditAnywhere, Category = "SL")
	bool bStartAtBeginPlay;

	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;


	/* Raw data logger properties */
	// Log raw data
	UPROPERTY(EditAnywhere, Category = "SL")
	bool bLogRawData;

	// Update rate of raw data logging (0.f means logging on every tick)
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float UpdateRate;

	// Distance (cm) threshold difference for logging a given item
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float DistanceThreshold;

	// Log data to json file
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	bool bLogToJson;

	// Log data to bson file
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	bool bLogToBson;

	// Log data to mongodb
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	bool bLogToMongo;

	// Mongodb server IP
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogToMongo"))
	FString MongoIP;

	// Mongodb server PORT
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data Logger", meta = (editcondition = "bLogToMongo"), meta = (ClampMin = 0, ClampMax = 65535))
	uint16 MongoPort;

	// Raw data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLRawDataLogger* RawDataLogger;



	/* Event data logger properties */
	// Log event data
	UPROPERTY(EditAnywhere, Category = "SL|Event Data Logger")
	bool bLogEventData;

	// Owl template
	UPROPERTY(EditAnywhere, Category = "SL|Event Data Logger")
	EEventsTemplate EventsTemplateType;

	// Event data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLEventDataLogger* EventDataLogger;
};
