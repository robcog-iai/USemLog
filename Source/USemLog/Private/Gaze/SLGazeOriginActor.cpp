// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Gaze/SLGazeOriginActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASLGazeOriginActor::ASLGazeOriginActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//SetMobility(EComponentMobility::Movable);

	//PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = false;

	//UpdateRate = 0.025f;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.25;
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLGazeOriginActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		AttachToActor(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager, FAttachmentTransformRules::SnapToTargetIncludingScale);
		//CameraManager = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager;
		//SetActorTickInterval(UpdateRate);
		//SetActorTickEnabled(true);
	}
}

// Called every frame
void ASLGazeOriginActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UE_LOG(LogTemp, Log, TEXT("%s::%d %s's Loc=%s; "), *FString(__FUNCTION__), __LINE__, *GetName(), *GetActorLocation().ToString());
	//Update();
}


//// Update its location according to the camera location
//void ASLGazeOriginActor::Update()
//{
//	SetActorTransform(CameraManager->GetTransform());
//	AttachToActor(CameraManager,FAttachmentTransformRules::SnapToTargetIncludingScale);
//}
