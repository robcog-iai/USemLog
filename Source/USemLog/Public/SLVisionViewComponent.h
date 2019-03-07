// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLVisionViewComponent.generated.h"

/**
 * Used for rendering pose in vision postprocessing
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName = "SL Vision View")
class USEMLOG_API USLVisionViewComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLVisionViewComponent();

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Init the component for runtime, returns true if it was already init, or init was successful
	bool Init();

	// Check if the component is init (and valid)
	bool IsInit() const { return bInit; };

private:
	// Clear data
	void ClearData();

	// Set the semantic parent and its data, returns true if successful or is already set
	bool SetSemanticOwnerAndData();

public:
	// Semantic owner
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	UObject* SemanticOwner;

	// Semantic data of the owner	
	FSLEntity OwnerSemanticData;

private:
	// Flag marking the component as init (and valid) for runtime 
	bool bInit;
};
