// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawData.h"
#include "TagStatics.h"

// Constructor
USLRawData::USLRawData()
{
}

// Destructor
USLRawData::~USLRawData()
{
}

// Read logging states (static/dynamic), write current world state
void USLRawData::Init(UWorld* InWorld)
{
	// Set the world
	World = InWorld;

	// Get the dynamic actors
	DynamicActors = FTagStatics::GetActorsWithKeyValuePair(
		World, "SemLog", "Runtime", "Dynamic");
}


// Log dynamic and static entities
void USLRawData::LogAllEntities()
{
	TArray<AActor*> StaticActors = FTagStatics::GetActorsWithKeyValuePair(
		World, "SemLog", "Runtime", "Static");

	for (const auto& ActItr : StaticActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("** Static actor: %s"), *ActItr->GetName());
	}

	USLRawData::LogDynamicEntities();
}

// Log dynamic entities
void USLRawData::LogDynamicEntities()
{
	for (const auto& ActItr : DynamicActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("** Dynamic actor: %s"), *ActItr->GetName());
	}
}