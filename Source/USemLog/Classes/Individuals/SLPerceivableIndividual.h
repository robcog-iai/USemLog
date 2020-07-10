// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLBaseIndividual.h"
#include "SLPerceivableIndividual.generated.h"

// Forward declarations
class UMaterialInstanceDynamic;
class UMaterialInterface;

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

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImport = false);

    // Save data to owners tag
    virtual bool ExportValues(bool bOverwrite = false) override;

    // Load data from owners tag
    virtual bool ImportValues(bool bOverwrite = false) override;

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("PerceivableIndividual"); };
    
    /* Begin Perceivable individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bPrioritizeChildren = false) { return false; };

    // Apply original materials
    virtual bool ApplyOriginalMaterials() { return false; };
    /* End Perceivable individual interface */

    // Toggle between the visual mask and the origina materials
    bool ToggleMaterials(bool bPrioritizeChildren = false);

    /* Visual mask */
    void SetVisualMaskValue(const FString& NewValue, bool bApplyNewMaterial = true, bool bClearCalibratedVisualMask = true);
    void ClearVisualMaskValue() { SetVisualMaskValue(""); };
    FString GetVisualMaskValue() const { return VisualMask; };
    bool IsVisualMaskValueSet() const { return !VisualMask.IsEmpty(); };

    /* Calibrated visual mask */
    void SetCalibratedVisualMaskValue(const FString& NewValue) { CalibratedVisualMask = NewValue; };
    void ClearCalibratedVisualMaskValue() { SetCalibratedVisualMaskValue(""); };
    FString GetCalibratedVisualMaskValue() const { return CalibratedVisualMask; };
    bool IsCalibratedVisualMasValueSet() const { return !CalibratedVisualMask.IsEmpty(); };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() const override;

    // Randomly generates a new visual mask, does not guarantee uniqueness
    FString GenerateNewRandomVisualMask() const;

    // Clear all values of the individual
    virtual void InitReset() override;

    // Clear all data of the individual
    virtual void LoadReset() override;

    // Clear any bound delegates (called when init is reset)
    virtual void ClearDelegateBounds() override;

private:
    // Set references
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport = true);

    // Specific imports from tag, true if new value is written
    bool ImportVisualMaskValue(bool bOverwrite = false);
    bool ImportCalibratedVisualMaskValue(bool bOverwrite = false);

    // Apply color to the dynamic material
    bool ApplyVisualMaskColorToDynamicMaterial();

    // Check if the dynmic material is valid
    bool HasValidDynamicMaterial() const;

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
    UMaterialInstanceDynamic* VisualMaskDynamicMaterial;

    // Cached original materials
    UPROPERTY()
    TArray<UMaterialInterface*> OriginalMaterials;
};
