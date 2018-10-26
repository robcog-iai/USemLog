// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisCamera.h"
#include "SLVisManager.h"


// Sets default values
ASLVisCamera::ASLVisCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Create the semantic Vis logging component
	SLVisManager = CreateDefaultSubobject<USLVisManager>(TEXT("SLVisManager"));
	SLVisManager->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ASLVisCamera::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ASLVisCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

