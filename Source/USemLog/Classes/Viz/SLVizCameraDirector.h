// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVizCameraDirector.generated.h"

// Forward declarations
class APawn;

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

	// Init director references
	void Init();

	// Move the camera position to the given pose (smooth movement if blend time is > 0)
	void MoveCameraTo(const FTransform& Pose, float BlendTime = -1.f);

	// Move and attach the camera to
	void AttachCameraTo(AActor* Actor, FName SocketName = NAME_None, float BlendTime = -1.f);

	// Make sure the camera is detached
	void DetachCamera();

private:
	// Start blend movement (set target and enable tick)
	void StartBlendMovement(const FTransform& Target, float BlendTime);

	// Stop blend movement (disable tick if active)
	void StopBlendMovement();

	// Update blend movement (interapolate and update pose)
	void UpdateBlendMovement(float DeltaTime);

	// Trigger test function
	void TriggerTest();

public:
	UPROPERTY(EditAnywhere, Category = Test)
	AActor* FirstViewTarget;

	UPROPERTY(EditAnywhere, Category = Test)
	AActor* SecondViewTarget;

	UPROPERTY(EditAnywhere, Category = Test)
	FTransform ViewTargetPose;

	UPROPERTY(EditAnywhere, Category = Test)
	AActor* FollowTarget;

	UPROPERTY(EditAnywhere, Category = Test)
	AActor* ThirdViewTarget;

	UPROPERTY(EditAnywhere, Category = Test)
	float TriggerDeltaTime = 4.5;

private:
	// True if the active pawn is set
	bool bIsInit;

	// Currently active pawn in the world
	APawn* ActivePawn;

	// View target to move the camera to
	FTransform BlendMoveToTarget;
};
