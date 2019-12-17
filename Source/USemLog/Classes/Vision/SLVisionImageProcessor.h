// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"

/**
 * 
 */
class FSLVisionImageProcessor
{
public:
	// Ctor
	FSLVisionImageProcessor();

	// Dtor
	~FSLVisionImageProcessor();

private:
	// Color to static mesh actor entity
	TMap<FColor, AStaticMeshActor*> MaskColorToStaticMeshActor;

};
