// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Demo/SLGazeVisualizer.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ASLGazeVisualizer::ASLGazeVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;
	// Disable tick by default, will be enabled if the eye tracking is succesfully init
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create the visual component
	GazeVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GazeVisual"));
	GazeVisual->SetMobility(EComponentMobility::Movable);
	RootComponent = GazeVisual;
	
	// Create the text component
	GazeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GazeText"));	
	GazeText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	GazeText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	GazeText->SetText("NONE");
	GazeText->SetWorldSize(1.f);
	GazeText->SetTextRenderColor(FColor::Red);
	GazeText->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>VisualAsset(TEXT("StaticMesh'/USemLog/Gaze/SM_GazeVisual.SM_GazeVisual'"));
	if (VisualAsset.Succeeded())
	{
		GazeVisual->SetStaticMesh(VisualAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialAsset(TEXT("StaticMesh'/USemLog/Gaze/M_GazeVisual.M_GazeVisual'"));
	if (MaterialAsset.Succeeded())
	{
		VisualMaterial = MaterialAsset.Object;		
	}	
	
	// Default process ptr to none
	GazeTraceFuncPtr = &ASLGazeVisualizer::GazeTrace_NONE;
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

	// Load the semantic data of all actors
	LoadSemanticData();

	// Bind trace function
	if (RayRadius <= KINDA_SMALL_NUMBER)
	{
		GazeTraceFuncPtr = &ASLGazeVisualizer::GazeTrace_Line;
	}
	else
	{
		SphereShape.SetSphere(RayRadius);
		GazeVisual->SetWorldScale3D(FVector(RayRadius * 2.f));
		GazeTraceFuncPtr = &ASLGazeVisualizer::GazeTrace_Sphere;
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
		(this->*GazeTraceFuncPtr)(OriginLoc, TargetLoc);
	}
#endif // SL_WITH_EYE_TRACKING	

	UE_LOG(LogTemp, Warning, TEXT("%s::%d Log message %.2f"),
		*FString(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
}

// Load info from tags
void ASLGazeVisualizer::LoadSemanticData()
{
	for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
	{
		FSLGazeSemanticData Data;

	}

	for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
	{

	}
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
		ProcessGazeHit(HitResult);
	}
}

// Process the gaze trace as a line
void ASLGazeVisualizer::GazeTrace_Sphere(const FVector& Origin, const FVector& Target)
{
	FHitResult HitResult;
	if (GetWorld()->SweepSingleByChannel(HitResult, Origin, Target, FQuat::Identity, ECC_Pawn, SphereShape, TraceParams))
	{
		ProcessGazeHit(HitResult);
	}
}

// Print out the gaze hit info
void ASLGazeVisualizer::ProcessGazeHit(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.Actor.Get();
	if (PrevActor != HitActor)
	{
		// Move visual to the hit location
		SetActorLocation(HitResult.Location);
		GazeText->SetText(HitActor->GetName());
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Hit=%s "),
			*FString(__FUNCTION__), __LINE__, *HitActor->GetName());




		PrevActor = HitActor;
	}

}

