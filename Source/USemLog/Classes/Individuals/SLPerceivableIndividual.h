// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLBaseIndividual.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SLPerceivableIndividual.generated.h"

// Notify every time the visual mask changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSLIndividualNewVisualMaskSignature, USLPerceivableIndividual*, PerceivableIndividual, FString, NewVisualMask);

/**
 * 
 */
UCLASS(ClassGroup = SL, abstract)
class USEMLOG_API USLPerceivableIndividual : public USLBaseIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLPerceivableIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    //// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
    //virtual void PostInitProperties() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImportFromTags = false);

    // Save data to owners tag
    virtual bool ExportToTag(bool bOverwrite = false) override;

    // Load data from owners tag
    virtual bool ImportFromTag(bool bOverwrite = false) override;

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("PerceivableIndividual"); };
    
    /* Begin Perceivable individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bReload = false) { return false; };

    // Apply original materials
    virtual bool ApplyOriginalMaterials() { return false; };
    /* End Perceivable individual interface */

    // Toggle between the visual mask and the origina materials
    bool ToggleMaterials();

    /* Visual mask */
    void SetVisualMask(const FString& NewVisualMask, bool bApplyNewMaterial = true, bool bClearCalibratedVisualMask = true);
    void ClearVisualMask() { SetVisualMask(""); };
    FString GetVisualMask() const { return VisualMask; };
    bool HasVisualMask() const { return !VisualMask.IsEmpty(); };

    /* Calibrated visual mask */
    void SetCalibratedVisualMask(const FString& NewCalibratedVisualMask) { CalibratedVisualMask = NewCalibratedVisualMask; };
    void ClearCalibratedVisualMask() { SetCalibratedVisualMask(""); };
    FString GetCalibratedVisualMask() const { return CalibratedVisualMask; };
    bool HasCalibratedVisualMask() const { return !CalibratedVisualMask.IsEmpty(); };

protected:
    // Clear all values of the individual
    virtual void InitReset() override;

    // Clear all data of the individual
    virtual void LoadReset() override;

    // Clear any bound delegates (called when init is reset)
    virtual void ClearDelegateBounds() override;

private:
    // States implementations, set references and data
    bool InitImpl();
    bool LoadImpl(bool bTryImportFromTags = true);

    // Specific imports from tag, true if new value is written
    bool ImportVisualMaskFromTag(bool bOverwrite = false);
    bool ImportCalibratedVisualMaskFromTag(bool bOverwrite = false);

    // Apply color to the dynamic material
    bool ApplyVisualMaskColorToDynamicMaterial();

    // Check if the dynmic material is valid
    bool HasValidDynamicMaterial() const 
    {
        return VisualMaskDynamicMaterial && VisualMaskDynamicMaterial->IsValidLowLevel() && !VisualMaskDynamicMaterial->IsPendingKill();
    };

public:
    // Called when the init status changes
    FSLIndividualNewVisualMaskSignature OnNewVisualMaskValue;

protected:
    // Mask color as hex
    UPROPERTY(VisibleAnywhere, Category = "SL")
    FString VisualMask;

    // Runtime calibrated mask color as hex
    UPROPERTY(VisibleAnywhere, Category = "SL")
    FString CalibratedVisualMask;

    // True if the visual masks are currently active on the semantic owner
    UPROPERTY(VisibleAnywhere, Category = "SL")
    bool bIsMaskMaterialOn;

    // Material template used for creating dynamic materials
    UPROPERTY()
    class UMaterial* VisualMaskMaterial;

    // Dynamic material used for setting various mask colors
    UPROPERTY()
    class UMaterialInstanceDynamic* VisualMaskDynamicMaterial;

    // Cached original materials
    UPROPERTY()
    TArray<class UMaterialInterface*> OriginalMaterials;
};
