// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizCameraDirector.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpectatorPawn.h"

// Sets default values
ASLVizCameraDirector::ASLVizCameraDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASLVizCameraDirector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLVizCameraDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);

	const float SmoothBlendTime = 0.75f;

	const float TimeBetweenCameraChanges = 1.0f;

	TimeToNextCameraChange -= DeltaTime;
	if (TimeToNextCameraChange <= 0.0f)
	{
		TimeToNextCameraChange += TimeBetweenCameraChanges;

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			
			//if (CameraTwo && (OurPlayerController->GetViewTarget() == CameraOne))
			//{
			//	//Blend smoothly to camera two.
			//	OurPlayerController->SetViewTargetWithBlend(CameraTwo, SmoothBlendTime);
			//}
			//else if (CameraOne)
			//{
			//	//Cut instantly to camera one.
			//	OurPlayerController->SetViewTarget(CameraOne);
			//}

			UE_LOG(LogTemp, Error, TEXT("%s::%d SWITCH "), *FString(__FUNCTION__), __LINE__);


			FVector LocOffset(0, 0, 10);
			PC->GetPawnOrSpectator()->AddActorWorldOffset(LocOffset);
		}
	}

}

// Set camera view
bool ASLVizCameraDirector::SetCameraView(const FTransform& Pose)
{
	//if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	//{
	//	//PC->SetViewTarget(ReplayViewTarget);
	//	//PC->SetActorLocation()
	//}

	//if (ReplayViewTarget)
	//{
	//	if (FPC)
	//	{
	//		FPC->SetViewTarget(ReplayViewTarget);
	//	}
	//	else
	//	{
	//		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	//		{
	//			PC->SetViewTarget(ReplayViewTarget);
	//			FPC = PC;
	//		}
	//	}
	//}
	return false;
}

