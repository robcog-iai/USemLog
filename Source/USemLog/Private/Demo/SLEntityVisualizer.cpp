// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)


#include "Demo/SLEntityVisualizer.h"

// Sets default values
ASLEntityVisualizer::ASLEntityVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASLEntityVisualizer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLEntityVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// True if it should tick in the editor
bool ASLEntityVisualizer::ShouldTickIfViewportsOnly() const
{
	return false;
}

