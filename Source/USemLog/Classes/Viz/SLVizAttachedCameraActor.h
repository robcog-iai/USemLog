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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Execute attachment
	void AttachCamera();

private:
	// Attach to actor at begin play
	UPROPERTY(EditAnywhere, Category = "SLViz")
	//AActor* ActorToAttachTo;
	TSoftObjectPtr<AActor> ActorToAttachTo;

	// True if the attachment should happen after a delay
	UPROPERTY(EditAnywhere, Category = "SLViz")
	bool bDelayAttachment = false;

	// Delay value in seconds
	UPROPERTY(EditAnywhere, Category = "SLViz", meta = (editcondition = "bDelayAttachment"))
	float DelayValue = 0.1f;
};
