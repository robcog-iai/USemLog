// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionComponent.h"
#if WITH_EDITOR
#include "Components/ArrowComponent.h"
#endif // WITH_EDITOR

// Sets default values for this component's properties
USLVisionComponent::USLVisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

#if WITH_EDITOR
	// Location and orientation visualization of the component
	ArrowVis = CreateDefaultSubobject<UArrowComponent>(TEXT("SLVisionArrowComponent"));
	ArrowVis->SetupAttachment(this);
	ArrowVis->ArrowSize = 0.1f;
	ArrowVis->ArrowColor = FColor::Blue;
#endif // WITH_EDITOR

	// Default parameters
	bUseCustomCameraId = false;
	CameraId = "autogen";
	bUseCustomResolution = true;
	Width = 480;
	Height = 320;
	FOV = 90.0;
	UpdateRate = 0.f; // 0.f = update as often as possible (boils down to every tick)
	bCaptureColor = true;
	bCaptureColorFromViewport = false;
	bCaptureDepth = true;
	bCaptureMask = true;
	bCaptureNormal = true;
}


// Called when the game starts
void USLVisionComponent::BeginPlay()
{
	Super::BeginPlay();	
}


// Called every frame
void USLVisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Init component
void USLVisionComponent::Init()
{

}

// Start capturing
void USLVisionComponent::Start()
{

}

// Stop recording
void USLVisionComponent::Finish()
{

}

