// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisManager.h"
#if WITH_EDITOR
#include "Components/ArrowComponent.h"
#endif // WITH_EDITOR

// Sets default values for this component's properties
USLVisManager::USLVisManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// Disable tick, if needed, it will be enabled when the framerate is set
	PrimaryComponentTick.bStartWithTickEnabled = false;

#if WITH_EDITOR
	// Location and orientation visualization of the component
	ArrowVis = CreateDefaultSubobject<UArrowComponent>(TEXT("SLVisArrowComponent"));
	ArrowVis->SetupAttachment(this);
	ArrowVis->ArrowSize = 1.0f;
	ArrowVis->ArrowColor = FColor::Blue;
#endif // WITH_EDITOR

	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Default parameters
	CameraId = TEXT("autogen"); // TODO use tags
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

// Destructor
USLVisManager::~USLVisManager()
{
	if (!bIsFinished)
	{
		USLVisManager::Finish();
	}
}


// Called when the game starts
void USLVisManager::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void USLVisManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Call update on tick
	USLVisManager::Update();
}

// Init component
void USLVisManager::Init(const FString& InLogDir, const FString& InEpisodeId)
{
	if (!bIsInit)
	{
		LogDirectory = InLogDir;
		EpisodeId = InEpisodeId;
	
		// Mark manager as initialized
		bIsInit = true;
	}
}

// Start capturing
void USLVisManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (UpdateRate > 0.0f)
		{
			// Update logger on custom timer tick (does not guarantees the UpdateRate value,
			// since it will be eventually triggered from the game thread tick
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLVisManager::Update, UpdateRate, true);
		}
		else
		{
			// Update logger on tick (updates every game thread tick, update rate can vary)
			SetComponentTickEnabled(true);
		}
		
		// Mark manager as started
		bIsStarted = true;
	}
}

// Stop recording
void USLVisManager::Finish()
{
	if (bIsStarted || bIsInit)
	{
		// Stop update timer;
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}

		// Mark manager as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Called either from tick, or from the timer
void USLVisManager::Update()
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
}