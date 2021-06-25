// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "SLVizAttachedCineCameraActor.generated.h"

/**
 * 
 */
UCLASS()
class ASLVizAttachedCineCameraActor : public ACineCameraActor
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
