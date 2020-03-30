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

	// Dtor
	~USLIndividualComponent();

	// Called when a component is created(not loaded).This can happen in the editor or during gameplay
	virtual void OnComponentCreated() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Save data to owners tag
	void SaveToTag(bool bOverwrite = false);

	// Load data from owners tag
	void LoadFromTag(bool bOverwrite = false);

private:
	// Convert datat type object to the selected class type
	void DoConvertDataType();

private:
	// Semantic data
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USLIndividualBase* SemanticIndividualObject;

	// Manually convert datatype to the chosen type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	TSubclassOf<class USLIndividual> ConvertToSemanticIndividual;
	
	/* Button workarounds */
	// Ovewrite any changes
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bOverwriteEditChanges;

	// Save data to tag
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bSaveToTagButton;

	// Load data from tag
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bLoadFromTagButton;
};
