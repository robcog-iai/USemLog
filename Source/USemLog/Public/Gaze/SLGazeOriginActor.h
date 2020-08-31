// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLGazeOriginActor.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Gaze Origin Actor")
class USEMLOG_API ASLGazeOriginActor : public AInfo
{
	GENERATED_BODY()
	
	// Sets default values for this actor's properties
	ASLGazeOriginActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

//protected:
//	// Update its location according to the camera location
//	void Update();
//
//protected:
//	// Update rate to query gaze data
//	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
//	float UpdateRate;
//
//	// Used for getting the gaze origin point
//	class APlayerCameraManager* CameraManager;
};
