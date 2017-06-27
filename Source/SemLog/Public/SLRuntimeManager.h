// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLRawData.h"
#include "SLEventData.h"
#include "SLMap.h"
#include "SLRuntimeManager.generated.h"

UCLASS()
class SEMLOG_API ASLRuntimeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLRuntimeManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Log raw data
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data")
	bool bLogRawData;

	// Raw data logger
	UPROPERTY(EditAnywhere, Category = "SL|Raw Data", meta = (editcondition = "bLogRawData"))
	USLRawData* RawDataLogger;

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "SL|Event Data")
	bool bLogEventData;
	
	// Event data logger
	UPROPERTY(EditAnywhere, Category = "SL|Event Data", meta = (editcondition = "bLogEventData"))
	USLEventData* EventLogger;

	//// Map of actors to be logged to their unique name
	//TMap<AActor*, FString> ActorToId;

	// Log raw data
	UPROPERTY(EditAnywhere, Category = "SL")
	USLMap* Map;
};
