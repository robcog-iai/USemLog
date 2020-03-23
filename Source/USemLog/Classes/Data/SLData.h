// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLData.generated.h"

USTRUCT()
struct FSLSkeletalData
{
    GENERATED_BODY()

    // Skeletal body part individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    FString Id;

    // Skeletal body part individual class
    UPROPERTY(EditAnywhere, Category = "SL")
    FString Class;
};


/**
 * 
 */
USTRUCT()
struct FSLIndividualData
{
    GENERATED_BODY()

    // Individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    FString Id;

    // Idividual class
    UPROPERTY(EditAnywhere, Category = "SL")
    FString Class;

    // Visual mask color in hex
    UPROPERTY(EditAnywhere, Category = "SL")
    FString VisualMaskHex;

    // The rendered value of the visual mask hex
    UPROPERTY(EditAnywhere, Category = "SL")
    FString CalibratedVisualMaskHex;

    // Skeletal data of the individual
    UPROPERTY(EditAnywhere, Category = "SL")
    TArray<FSLSkeletalData> SkeletalData;
};
