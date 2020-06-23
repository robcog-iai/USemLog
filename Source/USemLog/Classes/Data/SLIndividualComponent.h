// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SLBaseIndividual.h"
#include "SLIndividualComponent.generated.h"

// Delegate notification when the component is being destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLComponentDestroyedSignature, USLIndividualComponent*, DestroyedComponent);

// Notify every time the init status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLComponentInitChangeSignature, USLIndividualComponent*, Component, bool, bNewInitVal);

// Notify every time the loaded status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLComponentLoadedChangeSignature, USLIndividualComponent*, Component, bool, bNewLoadedVal);


/**
* Component storing the semantic individual information of its owner
*/
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual Component")
class USEMLOG_API USLIndividualComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualComponent();

protected:
	// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
	virtual void OnRegister() override;

	// Called when a component is created (not loaded) (after post init).This can happen in the editor or during gameplay
	virtual void OnComponentCreated() override;

	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:
	// Call init on the individual
	bool Init(bool bReset = false);

	// Check if component and  is initialized
	bool IsInit() const { return bIsInit; };

	// Load individual
	bool Load(bool bReset = false, bool bTryImportFromTags = false);

	// Check if component is loaded
	bool IsLoaded() const { return bIsLoaded; };

	// Get the semantic individual object
	USLBaseIndividual* GetIndividualObject() const { return HasIndividual() ? IndividualObj : nullptr; };

	// Get the semantic individual using a cast class (nullptr if cast is unsuccessfull)
	template <typename ClassType>
	ClassType* GetCastedIndividualObject() const 
	{
		if (IndividualObj && IndividualObj->IsValidLowLevel())
		{
			return Cast<ClassType>(IndividualObj);
		}
		return nullptr;
	};


	/* Functionalities */
	// Save data to owners tag
	bool ExportToTag(bool bOverwrite = false);

	// Load data from owners tag
	bool ImportFromTag(bool bOverwrite = false);

	// Toggle between original and mask material is possible
	bool ToggleVisualMaskVisibility();

protected:
	// Set the init flag, broadcast on new value
	void SetIsInit(bool bNewValue, bool bBroadcast = true);

	// Set the loaded flag, broadcast on new value
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

private:
	// Create individual if not created and forward init call
	bool InitImpl(bool bReset = false);

	// Forward load call on individual
	bool LoadImpl(bool bReset = false, bool bTryImportFromTags = false);

	// Sync states with the individual
	bool BindDelegates();

	// Check if individual was created
	FORCEINLINE bool HasIndividual() const 	{ return IndividualObj && IndividualObj->IsValidLowLevel() && !IndividualObj->IsPendingKill(); };

	// Private init implementation
	bool CreateIndividual();

	// Called on individual init flag change
	UFUNCTION()
	void OnIndividualInitChange(USLBaseIndividual* Individual, bool bNewValue);

	// Called on individual loaded flag change
	UFUNCTION()
	void OnIndividualLoadedChange(USLBaseIndividual* Individual, bool bNewValue);

public:
	// Called when the component is destroyed
	FSLComponentDestroyedSignature OnDestroyed;

	// Called when the init status changes
	FSLComponentInitChangeSignature OnInitChanged;

	// Called when the init status changes
	FSLComponentInitChangeSignature OnLoadedChanged;

private:
	// Semantic data
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USLBaseIndividual* IndividualObj;

	// True if the individual is succesfully created and initialized
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True if the individual is succesfully created and loaded
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;
};
