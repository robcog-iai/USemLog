// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Runtime/SLSymbolicLogger.h"
#include "Individuals/SLIndividualManager.h"
#include "Utils/SLUuid.h"
#include "EngineUtils.h"
#include "TimerManager.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLSymbolicLogger::ASLSymbolicLogger()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default values
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	bUseIndependently = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLSymbolicLogger"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Force call on finish
ASLSymbolicLogger::~ASLSymbolicLogger()
{
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		FinishImpl(true);
	}
}

// Allow actors to initialize themselves on the C++ side
void ASLSymbolicLogger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (bUseIndependently)
	{
		InitImpl();
	}
}

// Called when the game starts or when spawned
void ASLSymbolicLogger::BeginPlay()
{
	Super::BeginPlay();
	if (bUseIndependently)
	{
		if (StartParameters.StartTime == ESLLoggerStartTime::AtBeginPlay)
		{
			StartImpl();
		}
		else if (StartParameters.StartTime == ESLLoggerStartTime::AtNextTick)
		{
			FTimerDelegate TimerDelegateNextTick;
			TimerDelegateNextTick.BindLambda([this] {StartImpl(); });
			GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
		}
		else if (StartParameters.StartTime == ESLLoggerStartTime::AfterDelay)
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegateDelay;
			TimerDelegateDelay.BindLambda([this] {StartImpl(); });
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, StartParameters.StartDelay, false);
		}
		else if (StartParameters.StartTime == ESLLoggerStartTime::FromUserInput)
		{
			SetupInputBindings();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger (%s) StartImpl() will not be called.."),
				*FString(__func__), __LINE__, *GetName());
		}
	}
}

// Called when actor removed from game or game ended
void ASLSymbolicLogger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bUseIndependently && !bIsFinished)
	{
		FinishImpl();
	}
}

// Init logger (called when the logger is synced externally)
void ASLSymbolicLogger::Init(const FSLSymbolicLoggerParams& InLoggerParameters,
	const FSLLoggerLocationParams& InLocationParameters)
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	LoggerParameters = InLoggerParameters;
	LocationParameters = InLocationParameters;
	InitImpl();
}

// Start logger (called when the logger is synced externally)
void ASLSymbolicLogger::Start()
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	StartImpl();
}

// Finish logger (called when the logger is synced externally) (bForced is true if called from destructor)
void ASLSymbolicLogger::Finish(bool bForced)
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	FinishImpl();
}

// Init logger (called when the logger is used independently)
void ASLSymbolicLogger::InitImpl()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!LocationParameters.bUseCustomTaskId)
	{
		LocationParameters.TaskId = FSLUuid::NewGuidInBase64Url();
	}

	if (!LocationParameters.bUseCustomEpisodeId)
	{
		LocationParameters.EpisodeId = FSLUuid::NewGuidInBase64Url();
	}

	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!IndividualManager->Load(false))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		return;
	}

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) succesfully initialized at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Start logger (called when the logger is used independently)
void ASLSymbolicLogger::StartImpl()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is already started.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is not initialized, cannot start.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) succesfully started at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Finish logger (called when the logger is used independently) (bForced is true if called from destructor)
void ASLSymbolicLogger::FinishImpl(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is already finished.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit && !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is not initialized nor started, cannot finish.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}


	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) succesfully finished at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Bind user inputs
void ASLSymbolicLogger::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(StartParameters.UserInputActionName, IE_Pressed, this, &ASLSymbolicLogger::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLSymbolicLogger::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLSymbolicLogger::StartImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] Symbolic logger (%s) started.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else if (bIsStarted && !bIsFinished)
	{
		ASLSymbolicLogger::FinishImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] Symbolic logger (%s) finished.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] Symbolic logger (%s) logger finished, or not initalized.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
}

// Get the reference or spawn a new individual manager
bool ASLSymbolicLogger::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLIndividualManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			IndividualManager = *Iter;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_IndividualManager");
	IndividualManager = GetWorld()->SpawnActor<ASLIndividualManager>(SpawnParams);
#if WITH_EDITOR
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
	return true;
}
