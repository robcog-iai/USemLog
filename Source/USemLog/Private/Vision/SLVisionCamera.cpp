// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionCamera.h"
#include "Camera/CameraComponent.h"

// Ctor
ASLVisionCamera::ASLVisionCamera()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// Scale the camera mesh size for visual purposes
	GetCameraComponent()->SetWorldScale3D(FVector(0.1));
#endif // WITH_EDITORONLY_DATA
}
