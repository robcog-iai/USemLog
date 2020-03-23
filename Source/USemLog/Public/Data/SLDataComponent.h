// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SLData.h"
#include "SLDataComponent.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Data Component")
class USEMLOG_API USLDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLDataComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Print stored data to string
	FString ToString() const;

	//// Print data to tag
	//void WriteToTag();

	//// Read data from tag
	//void ReadFromTag();

private:
	// Semantic data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FSLIndividualData SemanticData;

	//// Individual unique id
	//UPROPERTY(EditAnywhere, Category = "SL")
	//FString Id;

	//// Idividual class
	//UPROPERTY(EditAnywhere, Category = "SL")
	//FString Class;

	//// Visual mask color in hex
	//UPROPERTY(EditAnywhere, Category = "SL")
	//FString VisualMaskHex;

	//// The rendered value of the visual mask hex
	//UPROPERTY(EditAnywhere, Category = "SL")
	//FString CalibratedVisualMaskHex;
};
