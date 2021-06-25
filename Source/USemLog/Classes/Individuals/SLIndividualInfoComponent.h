// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualInfoComponent.generated.h"

// Forward declarations
class USLIndividualComponent;
class USLIndividualInfoTextComponent;
class USLBaseIndividual;

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
	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config. (IsTemplate() could be true)
	virtual void PostInitProperties() override;

	// Do any object-specific cleanup required immediately after loading an object. (called only once when loading the map)
	virtual void PostLoad() override;

	// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called (also called after changes done in the editor)
	virtual void OnRegister() override;

	// Initializes the component.
	virtual void InitializeComponent() override;

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
	bool Init(bool bReset);

	// Check if component is initialized
	bool IsInit() const { return bIsInit; };

	// Load individual
	bool Load(bool bReset);

	// Check if component is loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Listen to the individual object delegates
	bool Connect();

	// True if the component is listening to the individual object delegates (transient)
	bool IsConnected() const { return bIsConnected; };

	// Enable/disable tick
	void ToggleTick();

	// Toggle text visiblity
	void SetTextVisibility(bool bNewVisiblitity);
	//void ToggleTextVisibility();

	// Rotate component towards the screen
	bool OrientateTowardsViewer();
	void OrientateTowardsLocation(const FVector& Location);

protected:
	// Set the init flag, return true if the state change
	void SetIsInit(bool bNewValue, bool bBroadcast  = true);

	// Set the loaded flag
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Set the connected flag, broadcast on new value
	void SetIsConnected(bool bNewValue, bool bBroadcast = true);

private:
	// Private init implementation
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

	// Clear all references of the individual
	void InitReset();

	// Clear all data of the individual
	void LoadReset();

	// Clear any bound delegates (called when init is reset)
	void ClearDelegates();

	// Update info as soon as the individual changes their data
	bool BindDelegates();

	// Check if individual component is set
	bool HasValidIndividualComponent() const;

	// Set the individual component 
	bool SetIndividualComponent();

	// Set individual text component
	bool SetTextComponent();

	// Clear individual text component
	void ClearTextComponent();

	// Check if the individual text component is valid
	bool HasValidTextComponent();

	// Set text components for the children
	bool SetChildrenTextComponents();

	// Destroy any children text components
	void ClearChildrenTextComponents();

	// Remove all rows from the children text
	void ClearChildrenTextComponentsData();

	// Check if the info component has children text components
	bool HasChildrenTextComponents() const { return ChildrenTextComponents.Num() > 0; };

	// Check if the children are in sync with the individual components
	bool HasChildrenInSync() const;

	// Check if the component is in the view frustrum
	bool IsInViewFrustrum(FVector& OutViewLocation) const;

	// Scale the text relative to the distance towards the location
	void SetTextScale(const FVector& Location);

	/* Text values */
	// Set its own states as text values
	void SetOwnTextInfo();

	// Set individual component state as text values
	void SetIndividualComponentTextInfo();

	/* Delegate functions */
	// Called when individual component init value has changed
	UFUNCTION()
	void OnIndividualComponentInitChanged(USLIndividualComponent* IC, bool bNewVal);

	// Called when the individual component load value has changed
	UFUNCTION()
	void OnIndividualComponentLoadedChanged(USLIndividualComponent* IC, bool bNewVal);

	// Called when the individual component connected value has changed
	UFUNCTION()
	void OnIndividualComponentConnectedChanged(USLIndividualComponent* IC, bool bNewVal);

	// Called when the individual component values have changed
	UFUNCTION()
	void OnIndividualComponentValueChanged(USLIndividualComponent* IC, const FString& Key, const FString& NewValue);

	// Called when the indiviual component child values have changed
	UFUNCTION()
	void OnIndividualComponentChildValueChanged(USLIndividualComponent* IC, USLBaseIndividual* ConstraintIndividual1, const FString& Key, const FString& NewValue);

	// Called when the children number has changed
	UFUNCTION()
	void OnIndividualComponentChildrenNumChanged(USLIndividualComponent* IC, int32 NumChildren, int32 NumAttachableChildren);

	// Called when individual component is being destroyed
	UFUNCTION()
	void OnIndividualComponentDestroyed(USLIndividualComponent* IC);

	// Called when individual component delegates are cleared
	UFUNCTION()
	void OnIndividualComponentDelegatesCleared(USLIndividualComponent* IC);

public:
	// Called when the component is destroyed
	FSLIndividualInfoComponentDestroyedSignature OnDestroyed;

	// Called when the delegates are cleared
	FSLIndividualInfoComponentDelegatesClearedSignature OnDelegatesCleared;

protected:
	// True if the manager is init
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger|Individual Info")
	uint8 bIsInit : 1;

	// True if the manager is loaded
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger|Individual Info")
	uint8 bIsLoaded : 1;

	// True if listening to the individual components delegates
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger|Individual Info")
	uint8 bIsConnected : 1;

	// Pointer to the individual component of the same owner
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger|Individual Component"*/)
	USLIndividualComponent* IndividualComponent;

private:
	// Shows values as rendered text lines
	UPROPERTY(/*VisibleAnywhere, Category = "SL|Individual Info"*/)
	USLIndividualInfoTextComponent* TextComponent;

	UPROPERTY(/*VisibleAnywhere, Category = "SL|Individual Info"*/)
	TMap<USLBaseIndividual*, USLIndividualInfoTextComponent*> ChildrenTextComponents;

	/* Constants */	
	// Text scale clamps
	constexpr static float TextScaleMin = 0.25f;
	constexpr static float TextScaleMax = 2.f;

	//static constexpr char OwnTextInfoKey[] = "iic";
	static constexpr auto OwnTextInfoKey = TEXT("iic");
	//static constexpr char ICTextInfoKey[] = "ic";
	static constexpr auto ICTextInfoKey = TEXT("ic");
};
