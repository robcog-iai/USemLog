// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizAttachedCineCameraActor.h"

// Called when the game starts or when spawned
void ASLVizAttachedCineCameraActor::BeginPlay()
{
	Super::BeginPlay();
	if (ActorToAttachTo && ActorToAttachTo->IsValidLowLevel())
	{
		AttachToActor(ActorToAttachTo, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s ActorToAttachTo is not valid.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
}