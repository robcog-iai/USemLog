// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Demo/SLGazeVisualizer.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ASLGazeVisualizer::ASLGazeVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;
	// Disable tick by default, will be enabled if the eye tracking is succesfully init
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Default process ptr to none
	ProcessGazeFuncPtr = &ASLGazeVisualizer::GazeTrace_NONE;
}

// Called when the game starts or when spawned
void ASLGazeVisualizer::BeginPlay()
{
	Super::BeginPlay();

#if SL_WITH_EYE_TRACKING
	for (TActorIterator<ASLGazeProxy> GazeItr(GetWorld()); GazeItr; ++GazeItr)
	{
		GazeProxy = *GazeItr;
		break;
	}

	if (!GazeProxy || !GazeProxy->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No ASLGazeProxy found in the world, eye tracking will be disabled.."), *FString(__func__), __LINE__);
		return;
	}

	if (!GazeProxy->Start())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not start the eye tracking software.."), *FString(__func__), __LINE__);
		return;
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		CameraManager = PC->PlayerCameraManager;
		if (CameraManager)
		{
			TraceParams = FCollisionQueryParams(FName("EyeTraceParam"), true, CameraManager);
			SetActorTickEnabled(true);
		}
	}

	// Bind trace function
	if (RayRadius <= KINDA_SMALL_NUMBER)
	{
		ProcessGazeFuncPtr = &ASLGazeVisualizer::GazeTrace_Line;
	}
	else
	{
		SphereShape.SetSphere(RayRadius);
		ProcessGazeFuncPtr = &ASLGazeVisualizer::GazeTrace_Sphere;
	}

#else	
	UE_LOG(LogTemp, Error, TEXT("%s::%d Eye tracking module not enabled.."), *FString(__func__), __LINE__);
#endif // SL_WITH_EYE_TRACKING	
}

// Called every frame
void ASLGazeVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
#if SL_WITH_EYE_TRACKING
	// Get the gaze direction
	FVector RelativeGazeDirection;
	if (GazeProxy->GetRelativeGazeDirection(RelativeGazeDirection))
	{
		const FVector OriginLoc = CameraManager->GetCameraLocation();
		const FVector TargetLoc = CameraManager->GetCameraRotation().RotateVector(OriginLoc + RelativeGazeDirection * RayLength);
		(this->*ProcessGazeFuncPtr)(OriginLoc, TargetLoc);
	}
#endif // SL_WITH_EYE_TRACKING	

	UE_LOG(LogTemp, Warning, TEXT("%s::%d Log message %.2f"),
		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
}

// Process the gaze trace default
void ASLGazeVisualizer::GazeTrace_NONE(const FVector& Origin, const FVector& Target)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Gaze is not going to be processed.."), *FString(__FUNCTION__), __LINE__);
}

// Process the gaze trace default
void ASLGazeVisualizer::GazeTrace_Line(const FVector& Origin, const FVector& Target)
{
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Origin, Target, ECC_Pawn, TraceParams))
	{		
		ProcessGazeHit(HitResult.Actor.Get());
	}
}

// Process the gaze trace as a line
void ASLGazeVisualizer::GazeTrace_Sphere(const FVector& Origin, const FVector& Target)
{
	FHitResult HitResult;
	if (GetWorld()->SweepSingleByChannel(HitResult, Origin, Target, FQuat::Identity, ECC_Pawn, SphereShape, TraceParams))
	{
		ProcessGazeHit(HitResult.Actor.Get());
	}
}

// Print out the gaze hit info
void ASLGazeVisualizer::ProcessGazeHit(AActor* Actor)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Hit=%s "),
		*FString(__FUNCTION__), __LINE__, *Actor->GetName());
}

