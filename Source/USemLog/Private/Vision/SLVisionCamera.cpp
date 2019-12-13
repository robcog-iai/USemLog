// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionCamera.h"
#include "Camera/CameraComponent.h"
#include "Tags.h"

// Ctor
ASLVisionCamera::ASLVisionCamera()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// Scale the camera mesh size for visual purposes
	GetCameraComponent()->SetWorldScale3D(FVector(0.1));
#endif // WITH_EDITORONLY_DATA
}

// Get the semantic class name of the virtual camera
FString ASLVisionCamera::GetClassName()
{
	if(ClassName.IsEmpty())
	{
		ClassName =  FTags::GetValue(this, "SemLog", "Class");
	}
	return ClassName;
}

// Get the unique id of the virtual camera
FString ASLVisionCamera::GetId()
{
	if (Id.IsEmpty())
	{
		Id = FTags::GetValue(this, "SemLog", "Id");
	}
	return Id;
}
