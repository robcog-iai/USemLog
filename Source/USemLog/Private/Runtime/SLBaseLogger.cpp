// // Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Runtime/SLBaseLogger.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Utils
#include "Utils/SLUuid.h"

// Sets default values
ASLBaseLogger::ASLBaseLogger()
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
ASLBaseLogger::~ASLBaseLogger()
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);
	if (bUseIndependently)
	{
		if (!IsTemplate())
		{
			FinishImpl(true);
		}
	}
}

// Allow actors to initialize themselves on the C++ side
void ASLBaseLogger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Log, TEXT("%s::%d"), *FString(__FUNCTION__), __LINE__);
	if (bUseIndependently)
	{
		InitImpl();
	}
}

// Called when the game starts or when spawned
void ASLBaseLogger::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);
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
void ASLBaseLogger::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);
	Super::Tick(DeltaTime);
}

// Called when actor removed from game or game ended
void ASLBaseLogger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);
	Super::EndPlay(EndPlayReason);

	if (bUseIndependently)
	{
		FinishImpl();
	}
}

#if WITH_EDITOR
//// Called when a property is changed in the editor
//void ASLBaseLogger::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	// Get the changed property name
//	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
//		PropertyChangedEvent.Property->GetFName() : NAME_None;
//
//	/* Logger Properties */
//	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLBaseLogger, LocationParameters.bUseCustomTaskId))
//	{
//		if (LocationParameters.bUseCustomTaskId) { LocationParameters.TaskId = FSLUuid::NewGuidInBase64Url(); }
//		else { LocationParameters.TaskId = TEXT(""); };
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLBaseLogger, LocationParameters.bUseCustomEpisodeId))
//	{
//		if (LocationParameters.bUseCustomEpisodeId) { LocationParameters.EpisodeId = FSLUuid::NewGuidInBase64Url(); }
//		else { LocationParameters.EpisodeId = TEXT(""); };
//	}
//
//	/* Start Properties*/
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLBaseLogger, StartParameters.bStartAtBeginPlay))
//	{
//		if (StartParameters.bStartAtBeginPlay) { StartParameters.bStartAtNextTick = false; StartParameters.bStartWithDelay = false; StartParameters.bStartFromUserInput = false; }
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLBaseLogger, StartParameters.bStartAtNextTick))
//	{
//		if (StartParameters.bStartAtNextTick) { StartParameters.bStartAtBeginPlay = false; StartParameters.bStartWithDelay = false; StartParameters.bStartFromUserInput = false; }
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLBaseLogger, StartParameters.bStartWithDelay))
//	{
//		if (StartParameters.bStartWithDelay) { StartParameters.bStartAtBeginPlay = false; StartParameters.bStartAtNextTick = false; StartParameters.bStartFromUserInput = false; }
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLBaseLogger, StartParameters.bStartFromUserInput))
//	{
//		if (StartParameters.bStartFromUserInput) { StartParameters.bStartAtBeginPlay = false;  StartParameters.bStartWithDelay = false; StartParameters.bStartAtNextTick = false; }
//	}
//}

//// Called by the editor to query whether a property of this object is allowed to be modified.
//bool ASLBaseLogger::CanEditChange(const UProperty* InProperty) const
//{
//	// Get parent edit property
//	const bool ParentVal = Super::CanEditChange(InProperty);
//
//	// Get the property name
//	const FName PropertyName = InProperty->GetFName();
//
//	return ParentVal;
//}
#endif // WITH_EDITOR

// Bind user inputs
void ASLBaseLogger::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(StartParameters.UserInputActionName, IE_Pressed, this, &ASLBaseLogger::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLBaseLogger::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLBaseLogger::StartImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] %s started.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else if (bIsStarted && !bIsFinished)
	{
		ASLBaseLogger::FinishImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] %s finished.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] %s: Something went wrong, try again.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
}

