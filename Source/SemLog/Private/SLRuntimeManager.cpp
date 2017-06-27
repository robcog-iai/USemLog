// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "SLRuntimeManager.h"


// Sets default values
ASLRuntimeManager::ASLRuntimeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EventLogger = CreateDefaultSubobject<USLEventData>(TEXT("EventData"));
	RawDataLogger = CreateDefaultSubobject<USLRawData>(TEXT("RawData"));
	Map = CreateDefaultSubobject<USLMap>(TEXT("MapData"));

}

// Called when the game starts or when spawned
void ASLRuntimeManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLRuntimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

