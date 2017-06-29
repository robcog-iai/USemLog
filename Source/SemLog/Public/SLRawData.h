// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLRawData.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USLRawData : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLRawData();

	// Destructor
	~USLRawData();

	// Distance treshold (squared for faster comparions)
	UPROPERTY(EditAnywhere, Category = "SL")
	float SquaredDistanceThreshold;

	// Initialise the logger
	void Init(UWorld* InWorld);

	// Log dynamic and static entities
	void LogAllEntities();

	// Log dynamic entities
	void LogDynamicEntities();

private:
	// Pointer to the world
	UWorld* World;

	// Array of the dynamic actors
	TArray<AActor*> DynamicActors;
};
