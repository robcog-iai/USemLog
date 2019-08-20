// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Engine/StaticMeshActor.h"
#include "SLStructs.h"
#include "SLContactShapeInterface.generated.h"

UINTERFACE(Blueprintable)
class USLContactShapeInterface : public UInterface
{
	GENERATED_BODY()
};

class ISLContactShapeInterface
{
	GENERATED_BODY()

public:
	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	virtual void Init() = 0;

	// Start publishing overlap events, trigger currently overlapping objects
	virtual void Start() = 0;

	// Stop publishing overlap events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Convenient function to get the world
	FORCEINLINE virtual UWorld* GetWorldFromShape() const { return ShapeComponent ? ShapeComponent->GetWorld() : nullptr; };

#if WITH_EDITOR
	// Update bounds visual (red/green -- parent is not/is semantically annotated)
	// it is public so it can be accessed from the editor panel for updates
	virtual void UpdateVisualColor() = 0;
#endif // WITH_EDITOR

protected:
#if WITH_EDITOR
	// Load and apply cached parameters from tags
	virtual bool LoadShapeBounds() = 0;

	// Calculate and apply trigger area size
	virtual bool CalcShapeBounds() = 0;

	// Save current parameters to tags
	virtual bool StoreShapeBounds() = 0;
#endif // WITH_EDITOR

	// Publish currently overlapping components
	void TriggerInitialOverlaps();

	// Event called when something starts to overlaps this component
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Event called when something stops overlapping this component 
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:
	// Event called when a semantic overlap begins
	FSLBeginContactSignature OnBeginSLContact;

	// Event called when a semantic overlap ends
	FSLEndContactSignature OnEndSLContact;

protected:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Pointer to the given shape component
	UShapeComponent* ShapeComponent;

	// Pointer to the outer (owner) mesh component 
	UMeshComponent* OwnerMeshComp;

	// Semantic data of the owner
	FSLEntity SemanticOwner;

	/* Constants */
	constexpr static const char* TagTypeName = "SemLogColl";
};