// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineMeshComponent.h"
#include "SLIndividualPartInfoComponent.generated.h"

class USplineMeshComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USLIndividualPartInfoComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USLIndividualPartInfoComponent();

protected:
	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	virtual void PostInitProperties() override;

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
	// Connect to individual component sibling
	bool Init(bool bReset = false);
	bool IsInit() { return bIsInit; };

	// Load values from individual sibling
	bool Load(bool bReset = false);
	bool IsLoaded() const { return bIsLoaded; };

	// Hide/show visual info
	void ToggleVisibility();
	void HideVisualInfo();
	void ShowVisualInfo();

protected:
	// Set the init flag, return true if the state change
	void SetIsInit(bool bNewValue, bool bBroadcast  = true);

	// Set the loaded flag
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Create spline mesh component (can be called outside of constructor)
	USplineMeshComponent* CreateSplineMeshComponent(class USLIndividualVisualAssets* Assets = nullptr);

private:
	// Private init implementation
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

	// Update info as soon as the individual changes their data
	bool BindDelegates();

protected:
	// Individual sibling is set
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsInit : 1;

	// Text data is loaded from sibling
	UPROPERTY(VisibleAnywhere, Category = "SL")
	uint8 bIsLoaded : 1;

private:
	/* Splines */
	//// Empty if individual is not part of another
	//UPROPERTY()
	//USplineMeshComponent* PartOfSplineMesh;	
};
