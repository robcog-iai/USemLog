// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLIndividualBase.h"
#include "SLIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USLIndividual : public USLIndividualBase
{
	GENERATED_BODY()
	
public:
	// Individual unique id
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Id;
	
	// Idividual class
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Class;
};
