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
	int32 Init(bool bReset = false);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

	// Re-load and re-register components
	int32 Reload() { return Init(true); };

	// Add new semantic data components to the actors
	int32 AddVisualInfoComponents();
	int32 AddVisualInfoComponents(const TArray<AActor*>& Actors);

	// Remove all components
	int32 DestroyVisualInfoComponents();
	int32 DestroyVisualInfoComponents(const TArray<AActor*>& Actors);

	// Refresh all components
	int32 RefreshVisualInfoComponents();
	int32 RefreshVisualInfoComponents(const TArray<AActor*>& Owners);

	// Toggle visiblity
	int32 ToggleVisualInfoComponents();
	int32 ToggleVisualInfoComponents(const TArray<AActor*>& Owners);

private:
	// Remove destroyed individuals from array
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualVisualInfoComponent* Component);

	// Find the text component of the actor, return nullptr if none found
	USLIndividualVisualInfoComponent* GetInfoComponent(AActor* Actor) const;

	// Create and add new individual component
	USLIndividualVisualInfoComponent* AddNewInfoComponent(AActor* Actor);

	// Check if an individual component is present
	bool CanHaveInfoComponent(AActor* Actor);

	// Remove individual component from owner
	void DestroyInfoComponent(USLIndividualVisualInfoComponent* Component);

	// Cache component, bind delegates
	bool RegisterInfoComponent(USLIndividualVisualInfoComponent* Component);

	// Remove component from cache, unbind delegates
	bool UnregisterInfoComponent(USLIndividualVisualInfoComponent* Component);

	// Unregister and clear all cached components (return the number of cleared components)
	int32 ClearCache();

private:
	// Marks manager as initialized
	bool bIsInit;

	// Cached components
	TSet<USLIndividualVisualInfoComponent*> RegisteredInfoComponents;
	TMap<AActor*, USLIndividualVisualInfoComponent*> InfoComponentOwners;
	TMap<USLIndividualVisualInfoComponent*, USLIndividualComponent*> InfoComponentsIndividuals;
};
