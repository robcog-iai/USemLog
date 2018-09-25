// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisActor.h"
#include "SLVisManager.h"


// Sets default values
ASLVisActor::ASLVisActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Create the semantic Vis logging component
	SLVisManager = CreateDefaultSubobject<USLVisManager>(TEXT("SLVisManager"));

	// Set as root component
	RootComponent = SLVisManager;
}

// Called when the game starts or when spawned
void ASLVisActor::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ASLVisActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

