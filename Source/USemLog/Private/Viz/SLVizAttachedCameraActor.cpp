// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizAttachedCameraActor.h"
#include "TimerManager.h"

// Called when the game starts or when spawned
void ASLVizAttachedCameraActor::BeginPlay()
{
	Super::BeginPlay();
	if (ActorToAttachTo && ActorToAttachTo->IsValidLowLevel())
	{
		if (bDelayAttachment)
		{
			FTimerHandle UnusedHandle;
			GetWorld()->GetTimerManager().SetTimer(UnusedHandle, this, 
				&ASLVizAttachedCameraActor::AttachCamera, DelayValue, false);
		}
		else 
		{
			AttachCamera();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s ActorToAttachTo is not valid.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
}

// Called every frame
void ASLVizAttachedCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Execute attachment
void ASLVizAttachedCameraActor::AttachCamera()
{
	AttachToActor(ActorToAttachTo.Get(), FAttachmentTransformRules::KeepWorldTransform);\

	if (AActor* AttParent = GetAttachParentActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d::%.4f camera %s succesfully attached to parent %s.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetName(), *ActorToAttachTo.Get()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d::%.4f Failed to attach camera %s to parent %s.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
			*GetName(), *ActorToAttachTo.Get()->GetName());
	}
}