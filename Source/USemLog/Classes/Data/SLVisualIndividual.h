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

    // Save data to owners tag
    virtual bool SaveToTag(bool bOverwrite = false) override;

    // Load data from owners tag
    virtual bool LoadFromTag(bool bOverwrite = false) override;

    // Set get visual mask
    void SetVisualMask(const FString& InVisualMask) { VisualMask = InVisualMask; };
    FString GetVisualMask() const { return VisualMask; };

    // Set get calibrated visual mask
    void SetCalibratedVisualMask(const FString& InCalibratedVisualMask) { CalibratedVisualMask = InCalibratedVisualMask; };
    FString GetCalibratedVisualMask() const { return CalibratedVisualMask; };

protected:
    // Skeletal body part individual unique id
    UPROPERTY(EditAnywhere, Category = "SL")
    FString VisualMask;

    // Skeletal body part individual class
    UPROPERTY(EditAnywhere, Category = "SL")
    FString CalibratedVisualMask;
};
