// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLVizQSemMap.generated.h"

// Forward declaration
class USLVizQBase;
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

public:
	virtual void Execute(ASLKnowrobManager* KRManager) override;

protected:
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

	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	bool bLoadButton = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	FString ArrayToLoadFromString;

	UPROPERTY(EditAnywhere, Category = "Semantic Map|Edit")
	bool bOverwrite = false;
};
