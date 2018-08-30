// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EventData/SLContactEvent.h"
#include "EventData/SLSupportedByEvent.h"
#include "SLContactTrigger.generated.h"

/** Delegate for notification of finished semantic contact event */
DECLARE_DELEGATE_OneParam(FSLContactEventSignature, TSharedPtr<FSLContactEvent>);

/** Delegate for notification of finished semantic supported by event */
DECLARE_DELEGATE_OneParam(FSLSupportedByEventSignature, TSharedPtr<FSLSupportedByEvent>);

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
	bool LoadAndApplyTriggerAreaSize();

	// Calculate and apply trigger area size
	bool CalculateAndApplyTriggerAreaSize();

	// Save parameters to tags
	bool StoreTriggerAreaSize(const FTransform& InTransform, const FVector& InBoxExtent);

	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	bool RuntimeInit();

	// Start new contact event
	void AddStartedContactEvent(
		const uint32 InOtherId,
		const FString& InOtherSemId,
		const FString& InOtherClass);

	// Start new supported by event
	void AddStartedSupportedByEvent(
		const uint32 InOtherId,
		const FString& InOtherSemId,
		const FString& InOtherClass);

	// Publish finished contact event
	bool FinishAndPublishContactEvent(const uint32 InOtherId);

	// Publish finished supported by event
	bool FinishAndPublishSupportedByEvent(const uint32 InOtherId);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAndPublishAllStartedEvents();

	// Check for supported by event
	bool IsASupportedByEvent(UPrimitiveComponent* InComp1, UPrimitiveComponent* InComp2);

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

public:
	// Event called when a semantic contact event is finished
	FSLContactEventSignature OnSemanticContactEvent;

	// Event called when a supported by event is finished
	FSLSupportedByEventSignature OnSemanticSupportedByEvent;

private:
	// Array of started contact events
	TArray<TSharedPtr<FSLContactEvent>> StartedContactEvents;

	// Candidates for supported by event
	TArray<TSharedPtr<FSLSupportedByEvent>> CandidatesForSupportedByEvent;

	// Array of started supported by events
	TArray<TSharedPtr<FSLSupportedByEvent>> StartedSupportedByEvents;

	// Pointer to the outer (owner) mesh actor
	AStaticMeshActor* OuterMeshAct;

	// Pointer to the outer (owner) mesh component 
	UStaticMeshComponent* OuterMeshComp;

	// Cache of the semlog id of the outer (owner)
	FString OuterSemId;

	// Cache of the semantic class of the outer (owner)
	FString OuterClass;

	// Cache of the outer (owner) unique id (unreal)
	uint32 OuterId;

	//// Bottom collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* BottomArea;

	//// Top collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* TopArea;	
};
