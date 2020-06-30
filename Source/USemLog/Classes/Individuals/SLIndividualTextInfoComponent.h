// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SLIndividualTextInfoComponent.generated.h"

// Forward declarations
class USLIndividualComponent;
class UTextRenderComponent;
class UMaterialInterface;

// Delegate notification when the component is being destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSLVisualInfoComponentDestroyedSignature, USLIndividualTextInfoComponent*, DestroyedComponent);

/**
* Component storing the visual information of semantic individuals
*/
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual Text Info")
class USEMLOG_API USLIndividualTextInfoComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualTextInfoComponent();

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

	// Point text towards the camera
	bool PointToCamera();

protected:
	// Set the init flag, return true if the state change
	void SetIsInit(bool bNewValue, bool bBroadcast = true);

	// Set the loaded flag
	void SetIsLoaded(bool bNewValue, bool bBroadcast = true);

	// Check if sibling component is set
	FORCEINLINE bool HasOwnerIndividualComponent() const
	{
		return OwnerIndividualComponent && OwnerIndividualComponent->IsValidLowLevel() && !OwnerIndividualComponent->IsPendingKill();
	};

	// Set the sibling component
	bool SetOwnerIndividualComponent();

	// Check if owner individual object is set
	FORCEINLINE bool HasOwnerIndividualObj() const
	{
		return OwnerIndividualObj && OwnerIndividualObj->IsValidLowLevel() && !OwnerIndividualObj->IsPendingKill();
	};

	// Set the owner individual object
	bool SetOwnerIndividualObj();

private:
	// Private init implementation
	bool InitImpl();

	// Private load implementation
	bool LoadImpl();

	// Update info as soon as the individual changes their data
	bool BindDelegates();

	// Set the color of the text depending on the owner individual component state;
	void SetTextColors();

	// Recalculate the size of the text
	void ResizeText();

	// Set the text values to default
	void ResetTextContent();

	// Render text subobject creation helper (should be called in constructor)
	UTextRenderComponent* CreateTextComponentSubobject(const FString& DefaultName, class USLIndividualVisualAssets* Assets = nullptr);

	// Create spline mesh component (can be called outside of constructor)
	USplineMeshComponent* CreateSplineMeshComponent(class USLIndividualVisualAssets* Assets = nullptr);

	/* Delegate functions */
	// Called when siblings init value has changed
	UFUNCTION()
	void OnOwnerIndividualComponentInitChanged(USLIndividualComponent* Component, bool bNewVal);

	// Called when the siblings load value has changed
	UFUNCTION()
	void OnOwnerIndividualComponentLoadedChanged(USLIndividualComponent* Component, bool bNewVal);

	// Called when sibling is being destroyed
	UFUNCTION()
	void OnOwnerIndividualComponentDestroyed(USLIndividualComponent* Component);

	// Called when the individual class value has changed
	UFUNCTION()
	void OnOwnerIndividualClassChanged(USLBaseIndividual* BI, const FString& NewVal);

	// Called when the individual id value has changed
	UFUNCTION()
	void OnOwnerIndividualIdChanged(USLBaseIndividual* BI, const FString& NewVal);

public:
	// Called when the component is destroyed
	FSLVisualInfoComponentDestroyedSignature OnDestroyed;

protected:
	// Pointer to the individual component of the same owner
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger"*/)
	USLIndividualComponent* OwnerIndividualComponent;

	// Pointer to the individual of the sibling component
	UPROPERTY(/*VisibleAnywhere, Category = "Semantic Logger"*/)
	USLBaseIndividual* OwnerIndividualObj;

	// Individual sibling is set
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// Text data is loaded from sibling
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsLoaded : 1;

private:
	/* Text */
	// First render text line (e.g class)
	UPROPERTY()
	UTextRenderComponent* FirstText;

	// Second render text line (e.g. id)
	UPROPERTY()
	UTextRenderComponent* SecondText;

	// Third render text line (e.g. type)
	UPROPERTY()
	UTextRenderComponent* ThirdText;

	// Text size template value 
	float TextSize;

	// These will be multiplied with the template value to get the final size
	float FirstLineTextSizeRatio;
	float SecondLineTextSizeRatio;
	float ThirdLineTextSizeRatio;

	/* Constants */
	// Clamp the template text size between these values
	constexpr static float MinClampTextSize = 3.f;
	constexpr static float MaxClampTextSize = 6.f;

	// SLIndividualVisualAssets'/USemLog/Individual/SLIndividualVisualAssets.SLIndividualVisualAssets'
	constexpr static TCHAR* AssetContainerPath =  TEXT("/USemLog/Individuals/SLIndividualVisualAssets");
	constexpr static TCHAR* TextMaterialPath = TEXT("Material'/USemLog/Individuals/M_InfoTextTranslucent.M_InfoTextTranslucent'");
};
