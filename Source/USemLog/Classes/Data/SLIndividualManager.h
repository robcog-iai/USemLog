// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLIndividualManager.generated.h"

// Forward declaration
class USLIndividualComponent;

UCLASS()
class USEMLOG_API ASLIndividualManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLIndividualManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Load components from world
	bool Init(bool bReset = false);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

private:
	// Remove destroyed individuals from array
	UFUNCTION()
	void OnIndividualDestroyed(USLIndividualComponent* Component);

private:
	// Marks manager as initialized
	bool bIsInit;

	// Array of all visual components
	TArray<USLIndividualComponent*> IndividualComponents;

};
