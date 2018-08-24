// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLRawDataLogger.h"
#include "SLEventDataLogger.h"
#include "SLManager.generated.h"

/**
 * Semantic logging manager (controls the logging in the world)
 */
UCLASS(hidecategories = (Actor, LOD, Cooking, Transform) )
class USEMLOG_API ASLManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLManager();

	// Destructor
	~ASLManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Init loggers
	void Init();

	// Start loggers
	void Start();

	// Finish loggers
	void Finish();
	
private:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif // WITH_EDITOR

private:
	/* Semantic logger */
	// Episode Id (be default will be auto generated)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString EpisodeId;

	// Log directory (or the database name if saving to mongodb)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString LogDirectory;

	// Start at load time
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtBeginPlay;

	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;


	/* Raw data logger properties */
	// Log raw data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogRawData;

	// Update rate of raw data logging (0.f means logging on every tick)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float UpdateRate;

	// Distance (cm) threshold difference for logging a given item
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float DistanceThreshold;

	// Log data to json file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	bool bLogToJson;

	// Log data to bson file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	bool bLogToBson;

	// Log data to mongodb
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogRawData"))
	bool bLogToMongo;

	// Mongodb server IP
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogToMongo"))
	FString MongoIP;

	// Mongodb server PORT
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Raw Data Logger", meta = (editcondition = "bLogToMongo"), meta = (ClampMin = 0, ClampMax = 65535))
	uint16 MongoPort;

	// Raw data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLRawDataLogger* RawDataLogger;


	/* Event data logger properties */
	// Log event data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger")
	bool bLogEventData;

	// Owl template
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Event Data Logger")
	ESLEventsTemplate EventsTemplateType;

	// Event data logger, use UPROPERTY to avoid GC
	UPROPERTY()
	USLEventDataLogger* EventDataLogger;
};
