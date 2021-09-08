// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Misc/SLLookAtAnim.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
USLLookAtAnim::USLLookAtAnim()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
}


// Called when the game starts
void USLLookAtAnim::BeginPlay()
{
	Super::BeginPlay();

	if (Init())
	{
		SetComponentTickEnabled(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s: could not init component.."), *FString(__FUNCTION__), __LINE__, *GetName());
	}
	
}


// Called every frame
void USLLookAtAnim::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ApplyLookAt();
}

// Check if parent and parameters are valid
bool USLLookAtAnim::Init()
{
	if (!LookAtActor || !LookAtActor->IsValidLowLevel())
	{
		if (SoftLookAtActor && SoftLookAtActor->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s: Using soft reference.."), *FString(__FUNCTION__), __LINE__, *GetName());
			LookAtActor = SoftLookAtActor.Get();
		}
	}


	if (LookAtActor && LookAtActor->IsValidLowLevel())
	{
		if (AStaticMeshActor* Act = Cast<AStaticMeshActor>(GetOwner()))
		{
			ParentSM = Act;
			if (ParentSM->GetRootComponent()->Mobility == EComponentMobility::Movable)
			{
				bIsInit = true;
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s: parent actor is not set to movable.."), *FString(__FUNCTION__), __LINE__, *GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s: parent is not a static mesh actor.."), *FString(__FUNCTION__), __LINE__, *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s: no actor given to follow.."), *FString(__FUNCTION__), __LINE__, *GetName());
	}
	return false;
}

// Apply look at movement
void USLLookAtAnim::ApplyLookAt()
{
	// Get the look at rotator
	FVector Start = ParentSM->GetActorLocation();
	FVector Target = LookAtActor->GetActorLocation();
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(Start, Target);
	FRotator TargetRot = ParentSM->GetActorRotation();
	if (bPitch)
	{
		TargetRot.Pitch = LookAtRot.Pitch;
	}
	if (bYaw)
	{
		TargetRot.Yaw = LookAtRot.Yaw;
	}
	ParentSM->SetActorRotation(TargetRot);
}

