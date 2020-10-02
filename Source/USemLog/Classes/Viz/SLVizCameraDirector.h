// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVizCameraDirector.generated.h"

UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Camera Director")
class ASLVizCameraDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVizCameraDirector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set camera view
	bool SetCameraView(const FTransform& Pose);

private:
	float TimeToNextCameraChange;
};
