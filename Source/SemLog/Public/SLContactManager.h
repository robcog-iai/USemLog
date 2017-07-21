// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLRuntimeManager.h"
#include "SLContactManager.generated.h"

/**
* Area type of the contact box
*/
UENUM()
enum class EContactAreaType : uint8
{
	Default			UMETA(DisplayName = "Default"),
	Top				UMETA(DisplayName = "Top"),
	Bottom			UMETA(DisplayName = "Bottom"),
	Wrapper			UMETA(DisplayName = "Wrapper")
};

/**
* Semantic logging of contact events
*/
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent))
class SEMLOG_API USLContactManager : public UBoxComponent
{
	GENERATED_BODY()

	// Constructor
	USLContactManager();

	// Destructor 
	~USLContactManager();

	// Called when spawned or level started
	virtual void BeginPlay() override;

	#if WITH_EDITOR 
	// Called when a property changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif 

	// The type of the contact area
	UPROPERTY(EditAnywhere, Category = SL)
	EContactAreaType AreaType;

	// The parent of the component
	UPROPERTY(EditAnywhere, Category = SL)
	AActor* ParentActor;

	// Static mesh component of the parent
	UPROPERTY(EditAnywhere, Category = SL)
	UStaticMeshComponent* ParentStaticMeshComponent;

	// Called on overlap begin events
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called on overlap end events
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	// Start contact event
	bool StartContactEvent(AActor* OtherActor);

	// Finish contact event
	bool FinishContactEvent(AActor* OtherActor);

	#if WITH_EDITOR 
	// Update contact area
	void UpdateContactArea();

	// Calculate surface area
	void CaclulateAreaAsTop();

	// Calculate inner area
	void CalculateAreaAsBottom();

	// Calculate wrapper area
	void CalculateAreaAsWrapper();
	#endif 
	
	// Individual name of the parent (Ns + Class + Id);
	FOwlIndividualName ParentIndividual;

	// Map of other actor to individual name of the contact event
	TMap<AActor*, TSharedPtr<FOwlNode>> OtherActorToEvent;

	// Semantic events runtime manager
	ASLRuntimeManager* SemLogRuntimeManager;
};