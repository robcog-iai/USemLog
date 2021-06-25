// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "SLVizAttachedCameraActor.generated.h"

/**
 * 
 */
UCLASS()
class ASLVizAttachedCameraActor : public ACameraActor
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Attach to actor at begin play
	UPROPERTY(EditAnywhere, Category = "SLViz")
	AActor* ActorToAttachTo;
};
