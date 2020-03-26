// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLVisualIndividual.h"
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

    // Check if the visual data is set
    FORCEINLINE bool IsSet() const { return !VisualMask.IsEmpty() && !CalibratedVisualMask.IsEmpty(); }
};

/**
 * 
 */
UCLASS()
class USEMLOG_API USLSkeletalIndividual : public USLVisualIndividual
{
	GENERATED_BODY()
	
public:
    // Skeletal body part individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    TMap<FName, FSLBoneIndividual> Bones;
};
