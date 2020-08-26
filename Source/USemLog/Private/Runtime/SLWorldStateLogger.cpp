// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLWorldStateLogger.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Utils/SLUuid.h"

// Sets default values
ASLWorldStateLogger::ASLWorldStateLogger()
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
#endif // WITH_EDITORONLY_DATA
}

// Force call on finish
ASLWorldStateLogger::~ASLWorldStateLogger()
{
	if (bUseIndependently)
	{
		if (!IsTemplate() && !bIsFinished)
		{
			FinishImpl(true);
		}
	}
}

// Allow actors to initialize themselves on the C++ side
void ASLWorldStateLogger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (bUseIndependently)
	{
		InitImpl();
	}
}

// Called when the game starts or when spawned
void ASLWorldStateLogger::BeginPlay()
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

// Called every frame
void ASLWorldStateLogger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when actor removed from game or game ended
void ASLWorldStateLogger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bUseIndependently && !bIsFinished)
	{
		FinishImpl();
	}
}

// Init logger (called when the logger is synced externally)
void ASLWorldStateLogger::Init(const FSLWorldStateLoggerParams& InLoggerParameters, const FSLLoggerLocationParams& InLocationParameters)
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	LoggerParameters = InLoggerParameters;
	LocationParameters = InLocationParameters;
	InitImpl();
}

// Start logger (called when the logger is synced externally)
void ASLWorldStateLogger::Start()
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	StartImpl();
}

// Finish logger (called when the logger is synced externally) (bForced is true if called from destructor)
void ASLWorldStateLogger::Finish(bool bForced)
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	FinishImpl();
}

// Init logger (called when the logger is used independently)
void ASLWorldStateLogger::InitImpl()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
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


	// Get access to the individual manager (spawn one if not available)

	// Connect to db

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) succesfully initialized at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Start logger (called when the logger is used independently)
void ASLWorldStateLogger::StartImpl()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is already started.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is not initialized, cannot start.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (StartParameters.bResetStartTime)
	{
		GetWorld()->TimeSeconds = 0.f;
	}

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) succesfully started at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Finish logger (called when the logger is used independently) (bForced is true if called from destructor)
void ASLWorldStateLogger::FinishImpl(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is already finished.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit || !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) is not initialized or started, cannot finish.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger (%s) succesfully finished at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Bind user inputs
void ASLWorldStateLogger::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(StartParameters.UserInputActionName, IE_Pressed, this, &ASLWorldStateLogger::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLWorldStateLogger::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLWorldStateLogger::StartImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] World state logger (%s) started.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else if (bIsStarted && !bIsFinished)
	{
		ASLWorldStateLogger::FinishImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] World state logger (%s) finished.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] World state logger (%s) Something went wrong, try again.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
}

