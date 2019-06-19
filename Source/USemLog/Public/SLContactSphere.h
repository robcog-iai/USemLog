// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SLStructs.h"
#include "SLContactSphere.generated.h"

/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics), DisplayName = "SL Contact Sphere")
class USEMLOG_API USLContactSphere : public USphereComponent
{
	GENERATED_BODY()
public:
	// Default constructor
	USLContactSphere();

	// Dtor
	~USLContactSphere();

	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	void Init();

	// Start publishing overlap events, trigger currently overlapping objects
	void Start();

	// Stop publishing overlap events
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Called at level startup
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
private:
	// UObject interface
	// Called after the C++ constructor and after the properties have been initialized
	virtual void PostInitProperties() override;

	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UObject interface

	// USceneComponent interface
	// Called when this component is moved in the editor
	virtual void PostEditComponentMove(bool bFinished) override;
	// End of USceneComponent interface

	// Load and apply cached parameters from tags
	bool LoadShapeBounds();

	// Calculate and apply trigger area size
	bool CalcShapeBounds();

	// Save current parameters to tags
	bool StoreShapeBounds();

public:
	// Update bounds visual (red/green -- parent is not/is semantically annotated)
	// it is public so it can be accessed from the editor panel for updates
	void UpdateVisualColor();
#endif // WITH_EDITOR

private:
	// Publish currently overlapping components
	void TriggerInitialOverlaps();

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
	// Event called when a semantic overlap begins
	FBeginSLContactSignature OnBeginSLContact;

	// Event called when a semantic overlap ends
	FEndSLContactSignature OnEndSLContact;

private:
	// True if initialized
	bool bIsInit;

	// True if started
	bool bIsStarted;

	// True if finished
	bool bIsFinished;

	// Init and start at begin play
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bStartAtBeginPlay;

#if WITH_EDITOR
	// Box extent scale factor (smaller will be chosen)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float SphereScaleFactor;

	// The box extent will be at least this big
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float SphereMinSize;

	// The box extent will be at most this big
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float SphereMaxSize;

	// Mimics a button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bReCalcShapeButton;
#endif // WITH_EDITOR

	// Pointer to the outer (owner) mesh component 
	UMeshComponent* OwnerMeshComp;

	// Semantic data of the owner
	FSLEntity SemanticOwner;

	/* Constants */
	constexpr static const char* TagTypeName = "SemLogColl";
};
