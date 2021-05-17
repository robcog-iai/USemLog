// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizCineCamManager.h"
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
ASLVizCineCamManager::ASLVizCineCamManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bIsInit = false;
}

// Called when the game starts or when sActivePawned
void ASLVizCineCamManager::BeginPlay()
{
	Super::BeginPlay();
	Init();	
}

// Called every frame
void ASLVizCineCamManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

// Init director references
void ASLVizCineCamManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already init.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// if (GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator())
	// {
		// bIsInit = true;
	// }
	// else
	// {
		// UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not access any active pawn.."), *FString(__FUNCTION__), __LINE__, *GetName());
	// }
}


