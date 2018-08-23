// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLContactTrigger.generated.h"

/** Delegate for notification of start of overlap with a semantic entity */
DECLARE_DELEGATE_TwoParams(FSLBeginContactSignature, AActor*, const FString&);
/** Delegate for notification of end of overlap with a semantic entity */
DECLARE_DELEGATE_TwoParams(FSLEndContactSignature, AActor*, const FString&);


/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent))
class USEMLOG_API USLContactTrigger : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Default constructor
	USLContactTrigger();

protected:
	// Called at level startup
	virtual void BeginPlay() override;

private:
	// UObject interface
	// Called after the C++ constructor and after the properties have been initialized
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	// USceneComponent interface
	// Called when this component is moved in the editor
	virtual void PostEditComponentMove(bool bFinished);
#endif // WITH_EDITOR

	// Load and apply cached parameters from tags
	bool LoadStoredParameters();

	// Apply and save parameters to tags
	bool ComputeAndStoreParameters(UStaticMeshComponent* SMComp);

	// Get the outer (owner) mesh component (can be nullptr)
	UStaticMeshComponent* GetOuterMesh();

	// Get Id of outer (owner)
	FString GetOuterSemLogId() const;
	
	// Event called when something starts to overlaps this component
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	//// Event called when a component hits (or is hit by) something solid
	//UFUNCTION()
	//void OnHit(UPrimitiveComponent* HitComponent,
	//	AActor* OtherActor,
	//	UPrimitiveComponent* OtherComp,
	//	FVector NormalImpulse,
	//	const FHitResult& Hit);


public:
	// Event called when two semantically annotated items are colliding
	FSLBeginContactSignature OnBeginSemanticContact;

	// Event called when two semantically annotated items end colliding
	FSLEndContactSignature OnEndSemanticContact;

private:
	// Setup pointers to outer, check if semantically annotated
	bool Init();

	// Pointer to the outer (owner) mesh actor
	AStaticMeshActor* OuterMeshAct;

	// Pointer to the outer (owner) mesh component 
	UStaticMeshComponent* OuterMeshComp;

	// Cache of the semlog id of the outer (owner)
	FString OuterSemLogId;

	// Cache of the outer (owner) unique id (unreal)
	uint32 OuterUniqueId;

	//// Bottom collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* BottomArea;

	//// Top collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* TopArea;	
};
