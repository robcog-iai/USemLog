// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLLookAtAnim.generated.h"

// Forward declaration
class AStaticMeshActor;

/**
 *
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName = "SL Look At Anim")
class USLLookAtAnim : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLLookAtAnim();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Check if parent and parameters are valid
	bool Init();

	// Check if component is initialized
	bool IsInit() const { return bIsInit; };

private:
	// Apply look at movement
	void ApplyLookAt();

private:
	// True if the individual is succesfully created and initialized
	bool bIsInit = false;

	// Actor to follow
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	AActor* LookAtActor;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TSoftObjectPtr<AActor> SoftLookAtActor;

	// Rotation axis
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bPitch = true;

	// Rotation axis
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bYaw = true;

	// Parent actor to move
	AStaticMeshActor* ParentSM;
};
