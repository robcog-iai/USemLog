// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Gaze/SLGazeTargetActor.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

#if SL_WITH_EYE_TRACKING
#include "SLGazeProxy.h"
#endif // SL_WITH_EYE_TRACKING

// Sets default values
ASLGazeTargetActor::ASLGazeTargetActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
		
	//DisableComponentsSimulatePhysics();
	//SetActorEnableCollision(ECollisionEnabled::NoCollision);

	VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_GazeVisualDebug"));
	VisualComponent->SetMobility(EComponentMobility::Movable);
	VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = VisualComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/USemLog/Gaze/SM_GazeVisual.SM_GazeVisual"));
	if (SphereVisualAsset.Succeeded())
	{
		VisualComponent->SetStaticMesh(SphereVisualAsset.Object);
	}

	GazeProxy = nullptr;

	// Default values
	bIsInit = false;
	bUseVisualDebugMesh = false;

	UpdateRate = 0.025f;
}

// Called when the game starts or when spawned
void ASLGazeTargetActor::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

// Called every frame
void ASLGazeTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Update();
}

// Connect to the gaze data
void ASLGazeTargetActor::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bUseVisualDebugMesh)
	{
		VisualComponent->SetVisibility(false);
	}

	if (UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		CameraManager = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager;
	}

#if SL_WITH_EYE_TRACKING
	for (TActorIterator<ASLGazeProxy> Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			GazeProxy = *Iter;
			if (GazeProxy->Start())
			{
				if (UpdateRate > 0)
				{
					SetActorTickInterval(UpdateRate);
				}
				SetActorTickEnabled(true);
				bIsInit = true;
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s' gaze proxy (%s) found and started.."),
					*FString(__FUNCTION__), __LINE__, *GetName(), *GazeProxy->GetName());
			}
		}
	}

	// Gaze proxy not found in the world, spawning one..
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_GazeProxy");
	GazeProxy = GetWorld()->SpawnActor<ASLGazeProxy>(SpawnParams);
#if WITH_EDITOR
	GazeProxy->SetActorLabel(TEXT("SL_GazeProxy"));
#endif // WITH_EDITOR

	if (GazeProxy && GazeProxy->IsValidLowLevel() && !GazeProxy->IsPendingKillOrUnreachable())
	{
		if (GazeProxy->Start())
		{
			SetActorTickInterval(UpdateRate);
			SetActorTickEnabled(true);
			bIsInit = true;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s' gaze proxy (%s) spawned and started.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *GazeProxy->GetName());
		}
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("%s::%d SL_WITH_EYE_TRACKING flag is set to 0, eye tracking is disabled.."), *FString(__FUNCTION__), __LINE__);
#endif // SL_WITH_EYE_TRACKING
}

// Update pose according to the gaze data
void ASLGazeTargetActor::Update()
{
#if SL_WITH_EYE_TRACKING
	FVector RelativeGazeDirection;
	if (GazeProxy->GetRelativeGazeDirection(RelativeGazeDirection))
	{
		const FVector RaycastOrigin = CameraManager->GetCameraLocation();
		const FVector RaycastTarget = CameraManager->GetCameraRotation().RotateVector(RaycastOrigin + RelativeGazeDirection * RayLength);

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName("SL_GazeTraceParams"), true, CameraManager);
		FHitResult HitResult;

		// Trace type
		if (RayRadius == 0.f)
		{
			if (GetWorld()->LineTraceSingleByChannel(HitResult, RaycastOrigin, RaycastTarget, ECC_Pawn, TraceParams))
			{
				SetActorLocation(HitResult.ImpactPoint);
				SetActorRotation(HitResult.ImpactNormal.ToOrientationQuat());
			}
		}
		else
		{
			FCollisionShape Sphere;
			Sphere.SetSphere(RayRadius);
			if (GetWorld()->SweepSingleByChannel(HitResult, RaycastOrigin, RaycastTarget, FQuat::Identity, ECC_Pawn, Sphere, TraceParams))
			{
				SetActorLocation(HitResult.ImpactPoint);
				SetActorRotation(HitResult.ImpactNormal.ToOrientationQuat());
			}
		}
	}
#endif // SL_WITH_EYE_TRACKING
}
