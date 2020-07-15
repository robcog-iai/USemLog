// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualInfoComponent.generated.h"

// Forward declarations
class USLIndividualComponent;
class USLIndividualTextComponent;

// Delegate notification when the component is being destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLIndividualInfoComponentDestroyedSignature, USLIndividualInfoComponent*, DestroyedComponent);

// Notify listeners that the delegates have been cleared
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLIndividualInfoComponentDelegatesClearedSignature, USLIndividualInfoComponent*, Component);

/**
* Component storing the visual information of semantic individuals
*/
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual Info")
class USEMLOG_API USLIndividualInfoComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualInfoComponent();

protected:
	// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
	virtual void OnRegister() override;

	// Called when a component is created (not loaded) (after post init).This can happen in the editor or during gameplay
	virtual void OnComponentCreated() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:
	// Call init on the individual
	bool Init(bool bReset = false);

	// Check if component is initialized
	bool IsInit() const { return bIsInit; };

	// Load individual
	bool Load(bool bReset = false);

	// Check if component is loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Listen to the individual object delegates
	bool Connect();

	// True if the component is listening to the individual object delegates (transient)
	bool IsConnected() const { return bIsConnected; };

	// Toggle text visiblity
	void ToggleTextVisibility();

	// Rotate component towards the screen
	bool OrientateTowardViewer();

protected:
	// Clear all references of the individual
	void InitReset();

	// Clear all data of the individual
	void LoadReset();

	// Clear any bound delegates (called when init is reset)
	void ClearDelegates();

	// Set the init flag, return true if the state change
	void SetIsInit(bool bNewValue, bool bBroadcast = true);

	// Set the loaded flag
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Set the connected flag, broadcast on new value
	void SetIsConnected(bool bNewValue, bool bBroadcast = true);

private:
	// Private init implementation
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

	// Update info as soon as the individual changes their data
	bool BindDelegates();

	// Check if sibling individual component is set
	bool HasValidSiblingIndividualComponent() const;

	// Set the sibling component
	bool SetSiblingIndividualComponent();

	// Check if the component is in the view frustrum
	bool IsInFrustrum() const;

	// Scale the text relative to the distance towards it
	void SetTextScale();

	/* Delegate functions */
	// Called when siblings init value has changed
	UFUNCTION()
	void OnSiblingIndividualComponentInitChanged(USLIndividualComponent* IC, bool bNewVal);

	// Called when the siblings load value has changed
	UFUNCTION()
	void OnSiblingIndividualComponentLoadedChanged(USLIndividualComponent* IC, bool bNewVal);

	// Called when the siblings connected value has changed
	UFUNCTION()
	void OnSiblingIndividualComponentConnectedChanged(USLIndividualComponent* IC, bool bNewVal);

	// Called when the siblings values have changed
	UFUNCTION()
	void OnSiblingIndividualComponentValueChanged(USLIndividualComponent* IC, const FString& NewKey, const FString& NewValue);

	// Called when sibling is being destroyed
	UFUNCTION()
	void OnSiblingIndividualComponentDestroyed(USLIndividualComponent* IC);

	// Called when sibling delegates are cleared
	UFUNCTION()
	void OnSiblingIndividualComponentDelegatesClearedDestroyed(USLIndividualComponent* IC);

public:
	// Called when the component is destroyed
	FSLIndividualInfoComponentDestroyedSignature OnDestroyed;

	// Called when the delegates are cleared
	FSLIndividualInfoComponentDelegatesClearedSignature OnDelegatesCleared;

protected:
	// Pointer to the individual component of the same owner
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USLIndividualComponent* SiblingIndividualComponent;

	// True if the manager is init
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the manager is loaded
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;

	// True if listening to the individual components delegates
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsConnected : 1;

private:
	// Shows values as rendered text lines
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USLIndividualTextComponent* TextComponent;
};
