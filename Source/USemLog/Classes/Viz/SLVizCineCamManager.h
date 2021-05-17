// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLVizCineCamManager.generated.h"

// Forward declarations
class APawn;

UCLASS(ClassGroup = (SL), DisplayName = "SL Viz CineCam Manager")
class ASLVizCineCamManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVizCineCamManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Init director references
	void Init();

private:
	// Trigger test function
	void TriggerTest();


private:
	// True if the active pawn is set
	bool bIsInit;
};
