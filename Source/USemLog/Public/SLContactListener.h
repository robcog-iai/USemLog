// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "SLContactListener.generated.h"

/** Delegate for notification of start of overlap with a semantic entity */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBeginSemanticContactSignature, AActor*, OtherActor, const FString&, Id);
/** Delegate for notification of end of overlap with a semantic entity */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEndSemanticContactSignature, AActor*, OtherActor, const FString&, Id);


/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent))
class USEMLOG_API USLContactListener : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Default constructor
	USLContactListener();

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
	FString GetOuterId() const;
	
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

	// Event called when a component hits (or is hit by) something solid
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	// Event called when the underlying physics objects is put to sleep
	UFUNCTION()
	void OnSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

	// Event called when the underlying physics objects is woken up
	UFUNCTION()
	void OnWake(UPrimitiveComponent* WakingComponent, FName BoneName);

public:
	// Event called when two semantically annotated items are colliding
	FBeginSemanticContactSignature OnBeginSemanticContact;

	// Event called when two semantically annotated items end colliding
	FEndSemanticContactSignature OnEndSemanticContact;

private:
	// Pointer to the outer (owner) mesh component 
	UStaticMeshComponent* OuterMeshComp;

	// Unique Id of the outer (owner)
	FString OuterId;

	//// Bottom collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* BottomArea;

	//// Top collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* TopArea;	
};
