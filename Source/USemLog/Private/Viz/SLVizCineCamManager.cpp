// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizCineCamManager.h"
#include "Camera/CameraActor.h"
#include "CineCameraActor.h"
#include "Camera/CameraComponent.h"
#include "CineCameraComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLVizCineCamManager::ASLVizCineCamManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bStartWithTickEnabled = false;

	bIsInit = false;
	bCameraEffectActive = false;
	bZoomIn = true;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLCineCamManager"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when sActivePawned
void ASLVizCineCamManager::BeginPlay()
{
	Super::BeginPlay();
	Init();	
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLVizCineCamManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* VizQ */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizCineCamManager, bResetButton))
	{
		bResetButton = false;
		AllCameras.Empty();
		CineCameras.Empty();
		CurrCamIdx = 0;
		bIsInit = false;
		Init();
	}
}
#endif // WITH_EDITOR

// Init director references
void ASLVizCineCamManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already init.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (bIncludeBasicCameras)
	{
		// Get all cinematic camera actors from the world
		for (TActorIterator<ACameraActor> CActItr(GetWorld()); CActItr; ++CActItr)
		{
			AllCameras.Add(*CActItr);
			UE_LOG(LogTemp, Log, TEXT("%s::%d Added %s"), *FString(__FUNCTION__), __LINE__, *CActItr->GetName());
		}

		if (AllCameras.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No cameras found.."), *FString(__FUNCTION__), __LINE__);
			return;
		}
	}
	else 
	{
		// Get all cinematic camera actors from the world
		for (TActorIterator<ACineCameraActor> CCActItr(GetWorld()); CCActItr; ++CCActItr)
		{
			CineCameras.Add(*CCActItr);
			UE_LOG(LogTemp, Log, TEXT("%s::%d Added %s"), *FString(__FUNCTION__), __LINE__, *CCActItr->GetName());
		}

		if (CineCameras.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No cine cameras found.."), *FString(__FUNCTION__), __LINE__);
			return;
		}
	}

	// Bine user input trigger
	SetupInputBindings();
}

// Setup user input bindings
void ASLVizCineCamManager::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{			
			if (!bActionInputBindingSet)
			{
				IC->BindAction(UserInputActionName, IE_Pressed, this, &ASLVizCineCamManager::SwitchCamera);
				IC->BindAction(UserInputActionEffectName, IE_Pressed, this, &ASLVizCineCamManager::ExecuteCameraEffect);
				bActionInputBindingSet = true;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access player controller (make sure it is not called before BeginPlay).."), *FString(__FUNCTION__), __LINE__);
	}
}

// Goto next camera
void ASLVizCineCamManager::SwitchCamera()
{
	if (bCameraEffectActive)
	{
		StopZoomEffect();
		bCameraEffectActive = false;
	}

	if (bIncludeBasicCameras)
	{
		if (AllCameras.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No cameras, this should not happen.."), *FString(__FUNCTION__), __LINE__);
			return;
		}

		CurrCamIdx++;
		if (!AllCameras.IsValidIndex(CurrCamIdx))
		{
			CurrCamIdx = 0;
		}

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetViewTarget(AllCameras[CurrCamIdx]);
			ActiveCameraComp = AllCameras[CurrCamIdx]->GetCameraComponent();
			if (ACineCameraActor* AsCineCA = Cast<ACineCameraActor>(AllCameras[CurrCamIdx]))
			{
				ActiveCineCameraComp = AsCineCA->GetCineCameraComponent();
			}
			else
			{
				ActiveCineCameraComp = nullptr;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("%s::%d CamIdx=%d; CamName=%s; CamCompName=%s;"),
			*FString(__FUNCTION__), __LINE__, CurrCamIdx, *AllCameras[CurrCamIdx]->GetName(), *ActiveCameraComp->GetName());
	}
	else
	{
		if (CineCameras.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No cine cameras, this should not happen.."), *FString(__FUNCTION__), __LINE__);
			return;
		}

		CurrCamIdx++;
		if (!CineCameras.IsValidIndex(CurrCamIdx))
		{
			CurrCamIdx = 0;
		}

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetViewTarget(CineCameras[CurrCamIdx]);
			ActiveCameraComp = CineCameras[CurrCamIdx]->GetCameraComponent();
			ActiveCineCameraComp = CineCameras[CurrCamIdx]->GetCineCameraComponent();
		}

		UE_LOG(LogTemp, Warning, TEXT("%s::%d CamIdx=%d; CamName=%s; CamCompName=%s;"),
			*FString(__FUNCTION__), __LINE__, CurrCamIdx, *CineCameras[CurrCamIdx]->GetName(), *ActiveCameraComp->GetName());
	}

#if UE_BUILD_DEBUG
	if (GEngine) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "CamIdx=%d; CamName=%s;",
			CurrCamIdx, *CineCameras[CurrCamIdx]->GetName());
	}
#endif
}

// Trigger effect on camera
void ASLVizCineCamManager::ExecuteCameraEffect()
{
	if (!bCameraEffectActive)
	{
		if (ActiveCameraComp && ActiveCineCameraComp)
		{
			StartZoomEffect();
			bCameraEffectActive = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d ActiveCineCameraComp is not set.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		StopZoomEffect();
		bCameraEffectActive = false;
	}
}

// Change the lens focal point to create zoom
void ASLVizCineCamManager::StartZoomEffect()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Camera effect exec.."), *FString(__FUNCTION__), __LINE__);
	//ActiveCameraComp->SetAspectRatio(0.5);
	ActiveCineCameraComp->SetAspectRatio(0.5);
	if (!EffectsTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(EffectsTimerHandle, this, &ASLVizCineCamManager::ZoomTimerCallback, 0.01f, true);
	}
}

// Change the lens focal point to create zoom
void ASLVizCineCamManager::StopZoomEffect()
{
	if (EffectsTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EffectsTimerHandle);
	}
}

// Zoom effect timer callback
void ASLVizCineCamManager::ZoomTimerCallback()
{
	float CurrFocalLength = ActiveCineCameraComp->CurrentFocalLength;
	if (bZoomIn && CurrFocalLength > 999)
	{
		bZoomIn = false;
	}

	if (!bZoomIn && CurrFocalLength < 8)
	{
		bZoomIn = true;
	}

	float Rate = bZoomIn ? 0.0005f : -0.0005f;
	float NewFocalLength = CurrFocalLength + CurrFocalLength * Rate;	
	ActiveCineCameraComp->SetCurrentFocalLength(NewFocalLength);
	
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Setting focal lenght to %f.."), *FString(__FUNCTION__), __LINE__, NewFocalLength);
}
