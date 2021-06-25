// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizCameraDirector.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpectatorPawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PawnMovementComponent.h"
#include "TimerManager.h"

// Sets default values
ASLVizCameraDirector::ASLVizCameraDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bIsInit = false;
}

// Called when the game starts or when sActivePawned
void ASLVizCameraDirector::BeginPlay()
{
	Super::BeginPlay();
	Init();	
}

// Called every frame
void ASLVizCameraDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
	UpdateBlendMovement(DeltaTime);
}

// Init director references
void ASLVizCameraDirector::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already init.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator())
	{
		ActivePawn = GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator();
		ActivePawn->DisableComponentsSimulatePhysics();
		ActivePawn->SetActorEnableCollision(ECollisionEnabled::NoCollision);
		bIsInit = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not access any active pawn.."), *FString(__FUNCTION__), __LINE__, *GetName());
	}
}

// Move the camera position to the given pose (smooth movement if blend time is > 0)
void ASLVizCameraDirector::MoveCameraTo(const FTransform& Pose, float BlendTime)
{
	// Remove any previous attachments
	DetachCamera();

	// Stop any active blend movement
	StopBlendMovement();

	if (BlendTime > 0)
	{
		StartBlendMovement(Pose, BlendTime);
	}
	else
	{
		ActivePawn->SetActorTransform(Pose);
	}

	// Alternative
	//	FViewTargetTransitionParams Params;
	//	Params.BlendTime = 1.f;
	//PCM->SetViewTarget(this, Params);

	//PC->Possess(ActivePawn);
}

// Move and attach the camera to
void ASLVizCameraDirector::AttachCameraTo(AActor* Actor, FName SocketName, float BlendTime)
{
	// Remove any previous attachments
	DetachCamera();

	// Stop any active blend movement
	StopBlendMovement();

	if (BlendTime > 0)
	{
		StartBlendMovement(Actor->GetActorTransform(), BlendTime);
	}
	else
	{
		ActivePawn->SetActorTransform(Actor->GetActorTransform());
	}
	ActivePawn->AttachToActor(Actor, FAttachmentTransformRules::KeepWorldTransform, SocketName);
}

// Make sure the camera is detached
void ASLVizCameraDirector::DetachCamera()
{
	ActivePawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}


// Start blend movement (set target and enable tick)
void ASLVizCameraDirector::StartBlendMovement(const FTransform& Target, float BlendTime)
{
	BlendMoveToTarget = Target;
	SetActorTickEnabled(true);
}

// Stop blend movement (disable tick if active)
void ASLVizCameraDirector::StopBlendMovement()
{
	if (IsActorTickEnabled())
	{
		SetActorTickEnabled(false);
	}
}

// Update blend movement (interapolate and update pose)
void ASLVizCameraDirector::UpdateBlendMovement(float DeltaTime)
{
	ActivePawn->SetActorTransform(BlendMoveToTarget);
	StopBlendMovement();
}

