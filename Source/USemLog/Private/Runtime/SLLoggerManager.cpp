// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Runtime/SLLoggerManager.h"
#include "Runtime/SLWorldStateLogger.h"
#include "Runtime/SLSymbolicLogger.h"

#include "Editor/SLSemanticMapWriter.h"
#include "Owl/SLOwlTaskStatics.h"
#include "Individuals/SLIndividualUtils.h"

#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

// Utils
#include "Utils/SLUuid.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLLoggerManager::ASLLoggerManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default values
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	bUseIndependently = false;
	bLogWorldState = false;
	bLogActionsAndEvents = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLLoggerManager"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Sets default values
ASLLoggerManager::~ASLLoggerManager()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
void ASLLoggerManager::PostLoad()
{
	Super::PostLoad();
}

// Allow actors to initialize themselves on the C++ side
void ASLLoggerManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (bUseIndependently)
	{
		Init();
	}
}


// Called when the game starts or when spawned
void ASLLoggerManager::BeginPlay()
{
	Super::BeginPlay();
	if (bUseIndependently)
	{
		if (StartParams.StartTime == ESLLoggerStartTime::AtBeginPlay)
		{
			Start();
		}
		else if (StartParams.StartTime == ESLLoggerStartTime::AtNextTick)
		{
			FTimerDelegate TimerDelegateNextTick;
			TimerDelegateNextTick.BindLambda([this] {Start(); });
			GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
		}
		else if (StartParams.StartTime == ESLLoggerStartTime::AfterDelay)
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegateDelay;
			TimerDelegateDelay.BindLambda([this] {Start(); });
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, StartParams.StartDelay, false);
		}
		else if (StartParams.StartTime == ESLLoggerStartTime::FromUserInput)
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
void ASLLoggerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish();
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLLoggerManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Logger Properties */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLLoggerManager, bWriteSemanticMapButton))
	{
		bWriteSemanticMapButton = false;
		bool bOverwriteSemMap = true;
		WriteSemanticMap(bOverwriteSemMap);		
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLLoggerManager, bWriteTaskButton))
	{
		bWriteTaskButton = false;
		bool bOverwriteTask = true;
		WriteTask(bOverwriteTask);
	}
}

#endif // WITH_EDITOR

// Internal init
void ASLLoggerManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (bLogWorldState)
	{
		if (!SetWorldStateLogger())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) could not set the world state logger, aborting init.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}
		else if(WorldStateLogger->IsRunningIndependently())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) world state logger (%s) is running independently, aborting init.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *WorldStateLogger->GetName());
			return;
		}

		WorldStateLogger->Init(WorldStateLoggerParams, LocationParams, DBServerParams);
		if (!WorldStateLogger->IsInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) world state logger (%s) could not be init, aborting init.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *WorldStateLogger->GetName());
			return;
		}
	}

	if (bLogActionsAndEvents)
	{
		if (!SetSymbolicLogger())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) could not set the symbolic logger, aborting init.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}
		else if (SymbolicLogger->IsRunningIndependently())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) symbolic logger (%s) is running independently, aborting init.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *SymbolicLogger->GetName());
			return;
		}

		SymbolicLogger->Init(SymbolicLoggerParams, LocationParams);
		if (!SymbolicLogger->IsInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) symbolic logger (%s) could not be init, aborting init.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *SymbolicLogger->GetName());
			return;
		}
	}


	bIsInit = true;
	UE_LOG(LogTemp, Log, TEXT("%s::%d Logger manager (%s) succesfully initialized at %f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Start logging
void ASLLoggerManager::Start()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is already started.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is not initialized, cannot start.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (StartParams.bResetStartTime)
	{
		GetWorld()->TimeSeconds = 0.f;
	}

	if (bLogWorldState)
	{
		WorldStateLogger->Start();
		if (!WorldStateLogger->IsStarted())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) world state logger (%s) could not be started, aborting start.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *WorldStateLogger->GetName());
			return;
		}
	}

	if (bLogActionsAndEvents)
	{
		SymbolicLogger->Start();
		if (!SymbolicLogger->IsStarted())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger manager (%s) symbolic logger (%s) could not be started, aborting start.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *SymbolicLogger->GetName());
			return;
		}
	}

	bIsStarted = true;
	UE_LOG(LogTemp, Log, TEXT("%s::%d Logger manager (%s) succesfully started at %f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Finish logging
void ASLLoggerManager::Finish(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is already finished.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit || !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is not initialized or started, cannot finish.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (bLogWorldState)
	{
		WorldStateLogger->Finish();
	}

	if (bLogActionsAndEvents)
	{
		SymbolicLogger->Finish();
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;

	if (GetWorld())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Logger manager (%s) succesfully finished at %f.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
	}
}

// Bind user inputs
void ASLLoggerManager::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(StartParams.UserInputActionName, IE_Pressed, this, &ASLLoggerManager::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLLoggerManager::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLLoggerManager::Start();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] Logger manager (%s) started.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else if (bIsStarted && !bIsFinished)
	{
		ASLLoggerManager::Finish();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] Logger manager (%s) finished.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] Logger manager (%s), something went wrong, try again.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
}

// Get the reference or spawn a new initialized world state logger
bool ASLLoggerManager::SetWorldStateLogger()
{
	if (WorldStateLogger && WorldStateLogger->IsValidLowLevel() && !WorldStateLogger->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLWorldStateLogger>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			WorldStateLogger = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_WorldStateLogger");
	WorldStateLogger = GetWorld()->SpawnActor<ASLWorldStateLogger>(SpawnParams);
#if WITH_EDITOR
	WorldStateLogger->SetActorLabel(TEXT("SL_WorldStateLogger"));
#endif // WITH_EDITOR
	return true;
}

// Get the reference or spawn a new initialized symbolic logger
bool ASLLoggerManager::SetSymbolicLogger()
{
	if (SymbolicLogger && SymbolicLogger->IsValidLowLevel() && !SymbolicLogger->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLSymbolicLogger>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			SymbolicLogger = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_SymbolicLogger");
	SymbolicLogger = GetWorld()->SpawnActor<ASLSymbolicLogger>(SpawnParams);
#if WITH_EDITOR
	SymbolicLogger->SetActorLabel(TEXT("SL_SymbolicLogger"));
#endif // WITH_EDITOR
	return true;
}

// Write semantic map owl file using the semantic map id
void ASLLoggerManager::WriteSemanticMap(bool bOverwrite)
{
	// Make sure all the sl individual values are exported to tags
	FSLIndividualUtils::ExportValues(GetWorld(), bOverwrite);

	// Map writer parameters
	FSLSemanticMapWriterParams Params;
	Params.Id = LocationParams.SemanticMapId;
	Params.Description = LocationParams.SemanticMapDescription;
	Params.TemplateType = ESLOwlSemanticMapTemplate::IAIKitchen;
	Params.Level = GetWorld()->GetMapName();
	Params.DirectoryPath = TEXT("/SL/Maps/");
	Params.bOverwrite = bOverwrite;

	// Write the data from the tags as an owl file
	FSLSemanticMapWriter SemMapWriter;
	SemMapWriter.WriteToFile(GetWorld(), Params);
}

// Write task owl file using the task id
void ASLLoggerManager::WriteTask(bool bOverwrite)
{
	TSharedPtr<FSLOwlTask> Task = FSLOwlTaskStatics::CreateDefaultTask(LocationParams.TaskId, "log", "ameva_log");
	Task->AddTaskIndividual(LocationParams.TaskDescription, LocationParams.SemanticMapId);
	FString TaskPath = FPaths::ProjectDir() + TEXT("/SL/Tasks/");
	FSLOwlTaskStatics::WriteToFile(Task, TaskPath, bOverwrite);
}

