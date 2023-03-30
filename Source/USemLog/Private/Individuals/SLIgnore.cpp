// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Individuals/SLIgnore.h"

// Sets default values for this component's properties
USLIgnore::USLIgnore()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USLIgnore::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USLIgnore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

