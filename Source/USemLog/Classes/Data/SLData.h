// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLData.generated.h"

/**
 *
 */
USTRUCT()
struct FSLVisualData
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


///**
// *
// */
//USTRUCT()
//struct FSLSkeletalData
//{
//    GENERATED_BODY()
//
//    // Skeletal body part individual unique id
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FString Id;
//
//    // Skeletal body part individual class
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FString Class;
//
//    // Visual mask color in hex
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FSLVisualData VisualData;
//};
//
//
///**
// * 
// */
//USTRUCT()
//struct FSLIndividual
//{
//    GENERATED_BODY()
//
//    // Individual unique id
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FString Id;
//
//    // Idividual class
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FString Class;
//
//    // Visual mask color in hex
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FSLVisualData VisualData;
//
//    // Skeletal data of the individual
//    UPROPERTY(EditAnywhere, Category = "SL")
//    TArray<FSLSkeletalData> SkeletalData;
//};
//
///**
// *
// */
//USTRUCT()
//struct FSLVisibleIndividualData : public FSLIndividual
//{
//    GENERATED_BODY()
//
//    // Visual mask color in hex
//    UPROPERTY(EditAnywhere, Category = "SL")
//    FSLVisualData VisualData2;
//};
//
///**
// *
// */
//USTRUCT()
//struct FSLSkeletalIndividualData : public FSLVisibleIndividualData
//{
//    GENERATED_BODY()
//
//    // Visual mask color in hex
//    UPROPERTY(EditAnywhere, Category = "SL")
//    TArray<FSLSkeletalData> SkeletalData2;
//};
