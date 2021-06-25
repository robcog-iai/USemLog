// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLGazeOriginActor.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Gaze Origin Actor")
class USEMLOG_API ASLGazeOriginActor : public AActor
{
	GENERATED_BODY()
	
	// Sets default values for this actor's properties
	ASLGazeOriginActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
