// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "SLVizQSemMap.generated.h"

// Forward declaration
class ASLKnowrobManager;

/**
 * 
 */
UCLASS()
class USLVizQSemMap : public USLVizQBase
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Virtual implementation of the execute function
	virtual void ExecuteImpl(ASLKnowrobManager* KRManager) override;

protected:
	/* Semantic map parameters */
	UPROPERTY(EditAnywhere, Category = "Semantic Map")
	bool bHide = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map")
	bool bAllIndividuals = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map", meta=(editcondition = "!bAllIndividuals"))
	TArray<FString> Ids;

	UPROPERTY(EditAnywhere, Category = "Semantic Map")
	bool bIterate = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map", meta = (editcondition = "bIterate"))
	float IterateInterval = 0.05f;


	/* Editor interaction */
	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	bool bAddSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	bool bRemoveSelectedButton = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	bool bOverwrite = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	bool bEnsureUniqueness = true;
};
