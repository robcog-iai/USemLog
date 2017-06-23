// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLContactTrigger.generated.h"

UENUM()
enum class EContactAreaType : uint8
{
	Init			UMETA(DisplayName = "Init"),
	Top				UMETA(DisplayName = "Top"),
	Bottom			UMETA(DisplayName = "Bottom"),
	Wrapper			UMETA(DisplayName = "Wrapper")
};

/**
* Semantic logging of contact events
*/
UCLASS(ClassGroup = SemanticLogger, meta = (BlueprintSpawnableComponent))
class SEMLOG_API USLContactTrigger : public UBoxComponent
{
	GENERATED_BODY()

	// Constructor
	USLContactTrigger();

	// Destructor 
	~USLContactTrigger();

	// Called when spawned or level started
	virtual void BeginPlay() override;

	// Setting contact area size depending on the selcted type
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// The type of the contact area
	UPROPERTY(EditAnywhere, Category = SemanticLogger)
	EContactAreaType AreaType;

	// The parent of the component
	UPROPERTY(EditAnywhere, Category = SemanticLogger)
	AActor* ParentActor;

	// Static mesh component of the parent
	UPROPERTY(EditAnywhere, Category = SemanticLogger)
	UStaticMeshComponent* ParentStaticMeshComponent;

	// Called on overlap begin events
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called on overlap end events
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	// Update contact area
	void UpdateContactArea();

	// Calculate surface area
	void CaclulateAreaAsTop();

	// Calculate inner area
	void CalculateAreaAsBottom();

	// Calculate wrapper area
	void CalculateAreaAsWrapper();
};