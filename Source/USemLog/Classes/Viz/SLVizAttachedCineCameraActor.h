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

	// Follow instead of attaching
	UPROPERTY(EditAnywhere, Category = "SLViz")
	bool bFollow = false;

	// True if the attachment should happen after a delay
	UPROPERTY(EditAnywhere, Category = "SLViz", meta = (editcondition = "!bFollow"))
	bool bDelayAttachment = false;

	// Delay value in seconds
	UPROPERTY(EditAnywhere, Category = "SLViz", meta = (editcondition = "bDelayAttachment"))
	float DelayValue = 0.1f;

	// Relative transform between the actor to follow and itself
	FTransform RelFollowTransform;

	bool bFollowInit = false;

};
