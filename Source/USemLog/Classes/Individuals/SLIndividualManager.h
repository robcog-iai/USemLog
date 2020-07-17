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
	// Set references
	bool Init(bool bReset = false);

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

	// 
	bool Load(bool bReset = false);

	// 
	bool IsLoaded() const { return bIsLoaded; };

	// Listen to individual component delegates
	bool Connect();

	// True if the manager is listening to the component delegates (transient)
	bool IsConnected() const { return bIsConnected; };

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
	bool HasCachedIndividualComponents() const;

	// Remove any chached individuals
	void ClearCachedIndividualComponents();

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
	TSet<USLIndividualComponent*> IndividualComponents;
};

