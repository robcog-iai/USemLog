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
	int32 Init(bool bReset = false);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

	// Re-load and re-register individual components
	void Refresh() { Init(true); };

	// Add new semantic data components to the actors in the world
	int32 AddIndividualComponents();

	// Add new semantic data components to the selected actors
	int32 AddIndividualComponents(const TArray<AActor*>& Actors);

	// Remove all semantic data components from the world
	int32 DestroyIndividualComponents();

	// Remove selected semantic data components
	int32 DestroyIndividualComponents(const TArray<AActor*>& Actors);

	// Reload components data
	int32 ReloadIndividualComponents();

	// Reload selected actor components data
	int32 ReloadIndividualComponents(const TArray<AActor*>& Actors);

private:
	// Triggered by external destruction to remove from cache
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualComponent* Component);

	// Find the individual component of the actor, return nullptr if none found
	USLIndividualComponent* GetIndividualComponent(AActor* Actor);

	// Create and add new individual component
	USLIndividualComponent* AddNewIndividualComponent(AActor* Actor);

	// Check if actor is supported for creating an individual component
	bool CanHaveIndividualComponents(AActor* Actor);

	// Remove individual component from owner
	void DestroyIndividualComponent(USLIndividualComponent* Component);

	// Cache component, bind delegates
	bool RegisterIndividualComponent(USLIndividualComponent* Component);

	// Remove component from cache, unbind delegates
	bool UnregisterIndividualComponent(USLIndividualComponent* Component);

	// Unregister and clear all cached components
	void ClearIndividualComponents();

	// Check if component is registered (one check)
	bool IsIndividualComponentRegisteredFast(USLIndividualComponent* Component) const
	{ return RegisteredIndividualComponents.Contains(Component); };

	// Check if component is registered (full check)
	bool IsIndividualComponentRegisteredFull(USLIndividualComponent* Component) const;

private:
	// Marks manager as initialized
	bool bIsInit;

	// Cached components
	TSet<USLIndividualComponent*> RegisteredIndividualComponents;
	TMap<AActor*, USLIndividualComponent*> IndividualComponentOwners;
};

