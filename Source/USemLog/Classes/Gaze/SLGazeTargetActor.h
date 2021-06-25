// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLGazeTargetActor.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Gaze Target Actor")
class USEMLOG_API ASLGazeTargetActor : public AActor
{
	GENERATED_BODY()
	
	// Sets default values for this actor's properties
	ASLGazeTargetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Get init state
	bool IsInit() const { return bIsInit; };

protected:
	// Update its location according to the gaze data
	void Update();

	// Listen if it can listen to gaze data
	void Init();

protected:
	// Update rate to query gaze data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate;

	// Use mesh to show the location of the actor at runtime
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseVisualDebugMesh;

	// Visual debug mesh component
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	class UStaticMeshComponent* VisualComponent;

	// True is it can
	bool bIsInit;

	// Used for getting the gaze origin point
	class APlayerCameraManager* CameraManager;

	// Custom made sranipal proxy to avoid compilation issues
	class ASLGazeProxy* GazeProxy;

	/* Constants */
	constexpr static float RayLength = 1000.f;
	constexpr static float RayRadius = 1.5f;
};
