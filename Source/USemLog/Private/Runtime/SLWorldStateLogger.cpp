// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)


#include "SLWorldStateLogger.h"
#include "EngineUtils.h"
#include "Utils/SLUuid.h"


// Sets default values
ASLWorldStateLogger::ASLWorldStateLogger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default values


#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

//// Called when an actor is done spawning into the world (from UWorld::SpawnActor), both in the editor and during gameplay
//void ASLWorldStateLogger::PostActorCreated()
//{
//	Super::PostActorCreated();
//
//	for (TActorIterator<ASLWorldStateLogger> Iter(GetWorld()); Iter; ++Iter)
//	{
//		// Take into account only valid actors, ignore self
//		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable() && (*Iter) != this)
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d There is already a world state logger in the world.."),
//				*FString(__FUNCTION__), __LINE__);
//			SetActorLabel(GetActorLabel() + "_DUPLICATE");
//
//			// Defer the actor destruction for next tick
//			//CurrWorld->GetTimerManager().SetTimerForNextTick([&](){ConditionalBeginDestroy();});
//
//			break;
//		}
//	}	
//}

// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
void ASLWorldStateLogger::PostLoad()
{
	Super::PostLoad();
}


// Called when the game starts or when spawned
void ASLWorldStateLogger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLWorldStateLogger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// External init (overwrite parameters)
void ASLWorldStateLogger::Init(const FSLWorldStateLoggerParams& InParams)
{
	Params = InParams;
	Init();
}

// Internal init
void ASLWorldStateLogger::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (Params.bStartIndependently)
	{
		if (!Params.bUseCustomTaskId)
		{
			Params.TaskId = FSLUuid::NewGuidInBase64Url();
		}

		if (!Params.bUseCustomEpisodeId)
		{
			Params.EpisodeId = FSLUuid::NewGuidInBase64Url();
		}
	}

	// Get access to the individual manager (spawn one if not available)

	// Connect to db

	bIsInit = true;
}

// Start logging
void ASLWorldStateLogger::Start()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already started.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, cannot start.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (Params.bResetStartTime)
	{
		GetWorld()->TimeSeconds = 0.f;
	}

	bIsStarted = true;
}

// Finish logging
void ASLWorldStateLogger::Finish()
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already finished.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit || !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized or started, cannot finish.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
}

