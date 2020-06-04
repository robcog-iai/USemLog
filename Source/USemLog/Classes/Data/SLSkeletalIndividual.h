// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLPerceivableIndividual.h"
#include "SLSkeletalIndividual.generated.h"


/**
 *
 */
USTRUCT()
struct FSLBoneIndividual
{
    GENERATED_BODY()

    // Skeletal body part individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    FString VisualMask;

    // Skeletal body part individual class
    UPROPERTY(EditAnywhere, Category = "SL")
    FString CalibratedVisualMask;    
};

/**
 * 
 */
UCLASS()
class USEMLOG_API USLSkeletalIndividual : public USLPerceivableIndividual
{
	GENERATED_BODY()

protected:
    // Skeletal body part individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    TMap<FName, FSLBoneIndividual> Bones;
};
