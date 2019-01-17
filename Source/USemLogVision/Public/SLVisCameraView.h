// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "SLVisCameraView.generated.h"

UCLASS(ClassGroup = (SL), hidecategories = (HLOD, Mobile, Cooking, AssetUserData), meta = (DisplayName = "SLVis Camera View"))
class USEMLOGVISION_API ASLVisCameraView : public ACameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVisCameraView();

	// Get camera label
	FString GetCameraLabel() const { return CameraLabel; };

private:
	// Unique name of the camera
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString CameraLabel;
};
