// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLRawData.h"
#include "SLEventData.h"
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
	UPROPERTY(EditAnywhere, Category = "SL")
	bool bLogRawData;

	// Raw data logger
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogRawData"))
	USLRawData* RawDataLogger;

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "SL")
	bool bLogEventData;
	
	// Event data logger
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogEventData"))
	USLEventData* EventLogger;
};
