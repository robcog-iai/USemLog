// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "SLCollisionListener.generated.h"

/**
 * Collision area listening for semantic collision events
 */
UCLASS(ClassGroup = SL, meta = (BlueprintSpawnableComponent))
class USEMLOG_API USLCollisionListener : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Default constructor
	USLCollisionListener();

private:
	// UObject interface
	// Called after the C++ constructor and after the properties have been initialized
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	// End of UObject interface

	// Read values from tags
	bool LoadParameters(const TArray<FName>& InTags, FVector& OutExtent, FTransform& OutRelTransf);

	// Save values to tags
	void SaveParameters(TArray<FName>& OutTags, const FVector& InExtent, const FTransform& InRelTransf);

private:
	//// Bottom collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* BottomArea;

	//// Top collision area
	//UPROPERTY(EditAnywhere, Category = "SL")
	//UBoxComponent* TopArea;	
};
