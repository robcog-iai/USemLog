// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionActor.h"
#include "SLVisionComponent.h"


// Sets default values
ASLVisionActor::ASLVisionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Create the semantic vision logging component
	SLVisionComponent = CreateDefaultSubobject<USLVisionComponent>(TEXT("SLVisionComponent"));

	// Set as root component
	RootComponent = SLVisionComponent;
}

// Called when the game starts or when spawned
void ASLVisionActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLVisionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

