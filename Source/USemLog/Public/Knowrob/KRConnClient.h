// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KRConnClient.generated.h"

// Forward declarations
class ASLIndividualManager;

/**
*
**/
UCLASS()
class USEMLOG_API AKRConnClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKRConnClient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set any references
	bool Init();

	// Clear any references
	void Reset();

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

private:
	// Set the individual manager
	bool SetIndividualManager();

private:
	// True if the manager is init
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the individuals manager is set and initialized
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIndividualManagerSet : 1;

	// Convenient access to the individuals in the world
	ASLIndividualManager* IndividualManager;
};
