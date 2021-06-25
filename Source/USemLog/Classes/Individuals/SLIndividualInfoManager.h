// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLIndividualInfoManager.generated.h"

// Forward declaration
class USLIndividualInfoComponent;

/**
* Manages all individual visual info
*/
UCLASS()
class USEMLOG_API ASLIndividualInfoManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLIndividualInfoManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// If true, actor is ticked even if TickType == LEVELTICK_ViewportsOnly
	virtual bool ShouldTickIfViewportsOnly() const override;

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

	// Enable/disable tick update
	void ToggleTickUpdate();

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

	// Load values
	bool LoadImpl();

	// Bind to the cached individual component delegates
	bool BindDelegates();

	// Remove bounds from the cached individuals
	bool UnbindDelegates();

	// Check if there are any cached elements
	bool HasCachedIndividualInfoComponents() const;

	// Remove any chached components
	void ClearCachedIndividualInfoComponents();

	/* Delegate functions */
	// Remove destroyed individuals from cache
	UFUNCTION()
	void OnIndividualInfoComponentDestroyed(USLIndividualInfoComponent* DestroyedComponent);

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
	TSet<USLIndividualInfoComponent*> IndividualInfoComponents;

	/* Buttons */
	UPROPERTY(EditAnywhere, Category = "Sematic Logger|Editor")
	bool bToggleTickUpdate;
};
