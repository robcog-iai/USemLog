// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "SLContactMonitorInterface.h"
#include "SLContactMonitorBox.generated.h"

/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent), hidecategories = (HLOD, Mobile, Cooking, Navigation, Physics), DisplayName = "SL Contact Monitor Box")
class USEMLOG_API USLContactMonitorBox : public UBoxComponent, public ISLContactMonitorInterface
{
	GENERATED_BODY()
public:
	// Default constructor
	USLContactMonitorBox();

	// Dtor
	~USLContactMonitorBox();

	/* Begin ISLContactMonitorInterface*/
	// Initialize trigger area for runtime, check if outer is valid and semantically annotated
	virtual void Init(bool bLogSupportedByEvents = true) override;

	// Start publishing overlap events, trigger currently overlapping objects
	virtual void Start() override;

#if WITH_EDITOR
	// Update bounds visual (red/green -- parent is not/is semantically annotated)
	// it is public so it can be accessed from the editor panel for updates
	virtual void UpdateVisualColor() override;
#endif // WITH_EDITOR
	/* End ISLContactMonitorInterface*/

protected:
	// Called at level startup
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
#if WITH_EDITOR
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

	/* Begin ISLContactMonitorInterface*/
	// Load and apply cached parameters from tags
	virtual bool LoadShapeBounds() override;

	// Calculate and apply trigger area size
	virtual bool CalcShapeBounds() override;

	// Save current parameters to tags
	virtual bool StoreShapeBounds() override;
	/* End ISLContactMonitorInterface*/
#endif // WITH_EDITOR

	// Set collision parameters such as object name and collision responses
	void SetCollisionParameters();

#if WITH_EDITORONLY_DATA
private:
	// Box extent scale factor (smaller will be chosen)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float BoxExtentScaleFactor;

	// The box extent will be at least this big
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float BoxExtentMin;

	// The box extent will be at most this big
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float BoxExtentMax;

	// Mimics a button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bReCalcShapeButton;
#endif // WITH_EDITORONLY_DATA
};
