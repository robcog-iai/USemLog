// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLIndividualManager.generated.h"

// Forward declaration
class USLIndividualComponent;
class USLBaseIndividual;

UCLASS(ClassGroup = (SL), DisplayName = "SL Individual Manager")
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
	// Set the individual references
	bool Init(bool bReset);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

	// Check if all individuals are loaded
	bool Load(bool bReset);

	// True if all individuals are loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Listen to individual component delegates
	bool Connect();

	// True if the manager is listening to the component delegates (transient)
	bool IsConnected() const { return bIsConnected; };

	// Get all the individual components
	const TArray<USLIndividualComponent*>& GetIndividualComponents() const { return IndividualComponents; };

	// Get all the individuals
	const TArray<USLBaseIndividual*>& GetIndividuals() const { return Individuals; };

	// Get the individual object from the unique id
	USLBaseIndividual* GetIndividual(const FString& Id);

	// Get the individual component from the unique id
	USLIndividualComponent* GetIndividualComponent(const FString& Id);

	// Get the individual component owner from the unique id
	AActor* GetIndividualActor(const FString& Id);

protected:
	// Clear all cached references
	void InitReset();

	// 
	void LoadReset();

	// Set the init flag, broadcast on new value
	void SetIsInit(bool bNewValue, bool bBroadcast  = true);

	// Set the loaded flag, broadcast on new value
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Set the connected flag, broadcast on new value
	void SetIsConnected(bool bNewValue, bool bBroadcast = true);

private:
	// Cache references
	bool InitImpl();

	// Load any values
	bool LoadImpl();

	// Bind to the cached individual component delegates
	bool BindDelegates();

	// Remove bounds from the cached individuals
	bool UnbindDelegates();

	// Check if there are any cached elements
	bool HasCache() const;

	// Remove any chached individuals
	void ClearCache();

	// Add individual info to cache
	void AddToCache(USLIndividualComponent* IC);

	// Triggered by external destruction of individual component
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualComponent* DestroyedComponent);


private:
	// True if the manager is init
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the manager is loaded
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;

	// True if listening to the individual components delegates
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsConnected : 1;

	// The individual components in the world
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TArray<USLIndividualComponent*> IndividualComponents;
	//TSet<USLIndividualComponent*> IndividualComponents;

	// All individuals
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TArray<USLBaseIndividual*> Individuals;
	//TSet<USLBaseIndividual*> IndividualComponents;

	/* Id based quick access mappings */
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<FString, USLBaseIndividual*> IdToIndividuals;

	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<FString, USLIndividualComponent*> IdToIndividualComponents;
};

