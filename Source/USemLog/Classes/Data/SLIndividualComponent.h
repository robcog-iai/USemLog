// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SLIndividualBase.h"
#include "SLIndividualComponent.generated.h"


UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), DisplayName = "SL Individual Component")
class USEMLOG_API USLIndividualComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLIndividualComponent();

protected:
	// Called when a component is created(not loaded).This can happen in the editor or during gameplay
	virtual void OnComponentCreated() override;

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
	// Convert datat type object to the selected class type
	void DoConvertDataType();

private:
	// Semantic data
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USLIndividualBase* SemanticIndividual;

	// Manually convert datatype to the chosen type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TSubclassOf<USLIndividual> ConvertToSemanticIndividual;
};
