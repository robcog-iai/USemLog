// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLRuntimeManager.h"
#include "SLFurnitureStateManager.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API ASLFurnitureStateManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLFurnitureStateManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Check furniture state
	void CheckFurnitureState();

	// Update rate
	UPROPERTY(EditAnywhere, Category = SL)
	float UpdateRate;

	// Timer for checking the furniture state
	FTimerHandle FurnitureStateTimerHandle;

	// Semantic events runtime manager
	ASLRuntimeManager* SemLogRuntimeManager;
};
