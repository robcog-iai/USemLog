// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualVisualInfo.generated.h"

// Forward declarations
class USLIndividualComponent;

// Delegate notification when the component is being destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLVisualInfoComponentDestroyedSignature, USLIndividualVisualInfo*, Self);

/**
* Component storing the visual information of semantic individuals
*/
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual Visual Info")
class USEMLOG_API USLIndividualVisualInfo : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualVisualInfo();

	// Connect to individual component sibling
	bool Init(bool bReset = false);

	// Check if component is visible
	bool IsInit() { return bIsInit; };

	// Refresh values from sibling component (returns false if component not init)
	bool Refresh();

	// Hide/show component
	bool ToggleVisibility();

	// Point text towards the camera
	bool UpdateOrientation();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
	virtual void OnRegister() override;

	// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	virtual void PostInitProperties() override;

	// Called before destroying the object.
	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Called when sibling is being destroyed
	UFUNCTION()
	void OnSiblingDestroyed(USLIndividualComponent* Component);

public:
	// Called when the component is destroyed
	FSLVisualInfoComponentDestroyedSignature OnSLComponentDestroyed;

protected:
	// Individual component sibling
	USLIndividualComponent* Sibling;

	// State of the individual
	bool bIsInit;

private:
	// Class text
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger"*/)
	class UTextRenderComponent* ClassText;

	// Id text
	UPROPERTY(/*EditAnywhere, Category = "Semantic Logger"*/)
	class UTextRenderComponent* IdText;
	
	// Type text
	//UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	//class UTextRenderComponent* TypeText;
	
	// Text sizes
	float ClassTextSize;
	float IdTextSize;
	//float TypeTextSize;

	// Used for getting the gaze origin point
	class APlayerCameraManager* CameraManager;
};
