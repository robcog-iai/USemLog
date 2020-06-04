// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLIndividualVisualInfoManager.generated.h"

// Forward declaration
class USLIndividualVisualInfoComponent;

/**
* Manages all individual visual info
*/
UCLASS()
class USEMLOG_API ASLIndividualVisualInfoManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLIndividualVisualInfoManager();

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

	// Refresh all components
	bool RefreshComponents();

	// Refresh only selected actors components
	bool RefreshSelected(const TArray<AActor*> Owners);

private:
	// Remove destroyed individuals from array
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualVisualInfoComponent* Component);

private:
	// Marks manager as initialized
	bool bIsInit;

	// Array of all visual components
	TArray<USLIndividualVisualInfoComponent*> VisualComponents;
};
