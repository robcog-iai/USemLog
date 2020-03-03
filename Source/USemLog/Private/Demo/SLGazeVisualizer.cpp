// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Demo/SLGazeVisualizer.h"
#include "EngineUtils.h"

// Sets default values
ASLGazeVisualizer::ASLGazeVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASLGazeVisualizer::BeginPlay()
{
	Super::BeginPlay();

#if SL_WITH_EYE_TRACKING
	for (TActorIterator<ASLGazeProxy> GazeItr(GetWorld()); GazeItr; ++GazeItr)
	{
		GazeProxy = *GazeItr;
		break;
	}

	if (!GazeProxy || !GazeProxy->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No ASLGazeProxy found in the world, eye tracking will be disabled.."), *FString(__func__), __LINE__);
		return;
	}

	if (!GazeProxy->Start())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not start the eye tracking software.."), *FString(__func__), __LINE__);
	}
#elif
	UE_LOG(LogTemp, Error, TEXT("%s::%d Eye tracking module not enabled.."), *FString(__func__), __LINE__);
#endif // SL_WITH_EYE_TRACKING
	
}

// Called every frame
void ASLGazeVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

