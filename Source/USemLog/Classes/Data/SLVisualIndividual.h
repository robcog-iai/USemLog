// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLIndividual.h"
#include "SLVisualIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USEMLOG_API USLVisualIndividual : public USLIndividual
{
	GENERATED_BODY()

public:
    // Skeletal body part individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    FString VisualMask;

    // Skeletal body part individual class
    UPROPERTY(EditAnywhere, Category = "SL")
    FString CalibratedVisualMask;
};
