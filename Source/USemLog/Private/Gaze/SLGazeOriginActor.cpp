// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Gaze/SLGazeOriginActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASLGazeOriginActor::ASLGazeOriginActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASLGazeOriginActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		AttachToActor(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager, FAttachmentTransformRules::SnapToTargetIncludingScale);
		//CameraManager = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager;
	}
}
