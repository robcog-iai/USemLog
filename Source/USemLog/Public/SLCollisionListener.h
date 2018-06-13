// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "SLCollisionListener.generated.h"

/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent))
class USEMLOG_API USLCollisionListener : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Default constructor
	USLCollisionListener();

private:
	// UObject interface
	// Called after the C++ constructor and after the properties have been initialized
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	
	// USceneComponent interface
	// Called when this component is moved in the editor
	virtual void PostEditComponentMove(bool bFinished);

	// Load cached parameters from tags, and apply them
	bool LoadAndApplyParameters();

	// Apply and save parameters to tags
	bool ApplyAndSaveParameters(UStaticMeshComponent* SMComp);

private:
	//// Bottom collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* BottomArea;

	//// Top collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* TopArea;	
};
