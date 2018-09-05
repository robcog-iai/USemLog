// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLContactPublisher.h"
#include "SLSupportedByPublisher.h"
#include "SLOverlapArea.generated.h"


/** Delegate to notify that a contact happened between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLBeginOverlapSignature, const uint32 /*OtherId*/, const FString& /*OtherSemId*/, const FString& /*OtherSemClass*/, float /*StartTime*/,  bool /*bIsSLOverlapArea*/);

/** Delegate to notify that a contact happened between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_FiveParams(FSLEndOverlapSignature, const uint32 /*OtherId*/, const FString& /*OtherSemId*/, const FString& /*OtherSemClass*/, float /*EndTime*/, bool /*bIsSLOverlapArea*/);

/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics))
class USEMLOG_API USLOverlapArea : public UBoxComponent
{
	GENERATED_BODY()

protected:
	// Give access to private data
	friend class FSLContactPublisher;
	friend class FSLSupportedByPublisher;

public:
	// Default constructor
	USLOverlapArea();

protected:
	// Called at level startup
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	bool ReadAndApplyTriggerAreaSize();

	// Calculate and apply trigger area size
	bool CalculateAndApplyTriggerAreaSize();

	// Save parameters to tags
	bool SaveTriggerAreaSize(const FTransform& InTransform, const FVector& InBoxExtent);

	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	bool RuntimeInit();

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

	//// Event called when a blocking hit event occurs
	//UFUNCTION()
	//void OnHit(UPrimitiveComponent* HitComponent,
	//	AActor* OtherActor,
	//	UPrimitiveComponent* OtherComp,
	//	FVector NormalImpulse,
	//	const FHitResult& Hit);

public:
	// Contact publisher
	TSharedPtr<FSLContactPublisher> SLContactPub;

	// Supported by event publisher
	TSharedPtr<FSLSupportedByPublisher> SLSupportedByPub;

private:
	// Event called when a semantic overlap begins
	FSLBeginOverlapSignature OnBeginSLOverlap;

	// Event called when a semantic overlap ends
	FSLEndOverlapSignature OnEndSLOverlap;

	// Listen for contact events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bListenForContactEvents;

	// Listen for supported by events
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bListenForSupportedByEvents;

	//// Listen for sliding events
	//UPROPERTY(EditAnywhere)
	//bool bListenForSlidingEvents;

	//// Listen for pushed by events
	//UPROPERTY(EditAnywhere)
	//bool bListenForPushedByEvents;

	// Pointer to the outer (owner) mesh actor
	AStaticMeshActor* OwnerAsStaticMeshAct;

	// Pointer to the outer (owner) mesh component 
	UStaticMeshComponent* OwnerStaticMeshComp;

	// Cache of the outer (owner) unique id (unreal)
	uint32 OwnerId;

	// Cache of the semlog id of the outer (owner)
	FString OwnerSemId;

	// Cache of the semantic class of the outer (owner)
	FString OwnerSemClass;
};
