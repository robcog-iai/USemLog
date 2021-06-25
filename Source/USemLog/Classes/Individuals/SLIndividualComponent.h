// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLIndividualComponent.generated.h"

// Forward declarations
class USLBaseIndividual;
class UDataAsset;

// Delegate notification when the component is being destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLComponentDestroyedSignature, USLIndividualComponent*, DestroyedComponent);

// Notify every time the init status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLComponentInitChangeSignature, USLIndividualComponent*, Component, bool, bNewInitVal);

// Notify every time the loaded status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLComponentLoadedChangeSignature, USLIndividualComponent*, Component, bool, bNewLoadedVal);

// Notify every time the connected status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLComponentConnectedChangeSignature, USLIndividualComponent*, Component, bool, bNewConnectedVal);

// Notify when the individual object changes any value
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSLComponentValueChangeSignature, USLIndividualComponent*, Component, const FString&, Key, const FString&, Value);

// Notify when and individual child changes any value
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSLComponentChildValueChangeSignature, USLIndividualComponent*, Component, USLBaseIndividual*, Child, const FString&, Key, const FString&, Value);

// Notify listeners that the individual has children
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSLComponentChildrenNumChangeSignature, USLIndividualComponent*, Component, int32, NumChildren, int32, NumAttachableChildren);

// Notify listeners that the delegates have been cleared
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLComponentDelegatesClearedSignature, USLIndividualComponent*, Component);

/**
* Component storing the semantic individual information of its owner
*/
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual")
class USEMLOG_API USLIndividualComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualComponent();

protected:
	// Do any object-specific cleanup required immediately after loading an object. (called only once when loading the map)
	virtual void PostLoad() override;

	// Called when a component is created (not loaded) (after post init).This can happen in the editor or during gameplay
	virtual void OnComponentCreated() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:
	// Call init on the individual
	bool Init(bool bReset = false);

	// Check if component is initialized
	bool IsInit() const { return bIsInit; };

	// Load individual
	bool Load(bool bReset = false, bool bTryImport = false);

	// Check if component is loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Listen to the individual object delegates
	bool Connect();

	// True if the component is listening to the individual object delegates (transient)
	bool IsConnected() const { return bIsConnected; };

	// Get the semantic individual object
	USLBaseIndividual* GetIndividualObject() const { return HasValidIndividual() ? IndividualObj : nullptr; };

	// Return const version of the children array
	const TArray<USLBaseIndividual*>& GetIndividualChildren() const { return IndividualChildren; };

	// Return const version of the attachable children map (individual to bone/socket name)
	const TMap<USLBaseIndividual*, FName>& GetAttachableIndividualChildren() const { return AttachableIndividualChildren; };

	// Get the semantic individual using a cast class (nullptr if cast is unsuccessfull)
	template <typename ClassType>
	ClassType* GetCastedIndividualObject() const 
	{
		return HasValidIndividual() ? Cast<ClassType>(IndividualObj) : nullptr;
	};

	// Check if the component has individual children
	bool HasIndividualChildren() const { return IndividualChildren.Num() > 0; };

	/* Functionalities */
	// Save data to owners tag
	bool ExportValues(bool bOverwrite);

	// Load data from owners tag
	bool ImportValues(bool bOverwrite);

	// Clear exported values
	bool ClearExportedValues();

	// Toggle between original and mask material is possible
	bool ToggleVisualMaskVisibility(bool bIncludeChildren);

	// Re-broadcast all available values
	bool TriggerIndividualValuesBroadcast();

	// Update individuals and their children cached poses
	bool UpdateCachedPoses();

	/* Values */
	/* Id */
	bool WriteId(bool bOverwrite);
	bool ClearId();

	/* Class */
	bool WriteClass(bool bOverwrite);
	bool ClearClass();

	/* Visual Mask */
	bool WriteVisualMask(const FString& Value, bool bOverwrite, const TArray<FString>& ChildrenValues = TArray<FString>());
	bool ClearVisualMask();

protected:
	// Clear all references of the individual
	void InitReset();

	// Clear all data of the individual
	void LoadReset();

	// Clear any bound delegates (called when init is reset)
	void ClearDelegates();

	// Set the init flag, broadcast on new value
	void SetIsInit(bool bNewValue, bool bBroadcast  = true);

	// Set the loaded flag, broadcast on new value
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Set the connected flag, broadcast on new value
	void SetIsConnected(bool bNewValue, bool bBroadcast = true);

	// Set any individual children if available
	void SetIndividualChildren();

	// Clear any cached individual children
	void ClearIndividualChildren();

private:
	// Create individual if not created and forward init call
	bool InitImpl();

	// Forward load call on individual
	bool LoadImpl(bool bTryImport = false);

	// Sync states with the individual
	bool BindDelegates();

	// Sync states with the individuals children
	bool BindChildrenDelegates();

	// Clear children delegates
	bool UnBindChildrenDelegates();

	// Check if individual object is valid
	bool HasValidIndividual() const;

	// Create the individual object
	bool CreateIndividual();

	// Triggered on individual init flag change
	UFUNCTION()
	void OnIndividualInitChange(USLBaseIndividual* Individual, bool bNewValue);

	// Triggered on individual loaded flag change
	UFUNCTION()
	void OnIndividualLoadedChange(USLBaseIndividual* Individual, bool bNewValue);

	// Triggered when an individual value is changed
	UFUNCTION()
	void OnIndividualNewValue(USLBaseIndividual* Individual, const FString& Key, const FString& NewValue);

	// Triggered when a child individual value is changed
	UFUNCTION()
	void OnChildIndividualNewValue(USLBaseIndividual* Individual, const FString& Key, const FString& NewValue);

	// Triggered when individual delegates are cleared (including this one)
	UFUNCTION()
	void OnIndividualDelegatesCleared(USLBaseIndividual* Individual);

public:
	// Called when the component is destroyed
	FSLComponentDestroyedSignature OnDestroyed;

	// Called when the init status changes
	FSLComponentInitChangeSignature OnInitChanged;

	// Called when the init status changes
	FSLComponentInitChangeSignature OnLoadedChanged;

	// Called when the connected status changes
	FSLComponentConnectedChangeSignature OnConnectedChanged;

	// Called when any value change from the individual object
	FSLComponentValueChangeSignature OnValueChanged;

	// Called when any child value change from the individual object
	FSLComponentChildValueChangeSignature OnChildValueChanged;

	// Called when the children are set
	FSLComponentChildrenNumChangeSignature OnChildrenNumChanged;

	// Called when the delegates are cleared
	FSLComponentDelegatesClearedSignature OnDelegatesCleared;

private:
	// True if the individual is succesfully created and initialized
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the individual is succesfully created and loaded
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;

	// True if the component is listening to the individual object delegates
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsConnected : 1;

	// Semantic data
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USLBaseIndividual* IndividualObj;

	// Individuals children
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TArray<USLBaseIndividual*> IndividualChildren;

	// Individuals attachable children
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<USLBaseIndividual*, FName> AttachableIndividualChildren;


	/* Editor button hacks */
	// Triggers a call to the indivduals and its children update cached pose
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bUpdateIndivdualsCachedPoseButtonHack;

public:
	// Semantic data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FString, UDataAsset*> OptionalDataAssets;

	/* Constants */
	// Skeletal data asset map key
	static constexpr auto SkelDataAssetKey = TEXT("SkeletalDataAsset");
};
