// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "SLVisibleIndividual.generated.h"

// Forward declarations
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMaterial;

/**
 * 
 */
UCLASS(ClassGroup = SL, abstract)
class USEMLOG_API USLVisibleIndividual : public USLBaseIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLVisibleIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Trigger values as new value broadcast
    virtual void TriggerValuesBroadcast() override;

    // Save data to owners tag
    virtual bool ExportValues(bool bOverwrite = false) override;

    // Load data from owners tag
    virtual bool ImportValues(bool bOverwrite = false) override;

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("VisibleIndividual"); };
    
    /* Begin Visible individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bIncludeChildren = false) { return false; };

    // Apply original materials
    virtual bool ApplyOriginalMaterials() { return false; };
    /* End Visible individual interface */

    // Toggle between the visual mask and the origina materials
    bool ToggleMaterials(bool bIncludeChildren = false);

    /* Visual mask */
    void SetVisualMaskValue(const FString& NewValue, bool bApplyNewMaterial = true, bool bClearCalibratedVisualMask = true);
    void ClearVisualMaskValue() { SetVisualMaskValue(""); };
    FString GetVisualMaskValue() const { return VisualMask; };
    bool IsVisualMaskValueSet() const { return !VisualMask.IsEmpty(); };

    /* Calibrated visual mask */
    void SetCalibratedVisualMaskValue(const FString& NewValue);
    void ClearCalibratedVisualMaskValue() { SetCalibratedVisualMaskValue(""); };
    FString GetCalibratedVisualMaskValue() const { return CalibratedVisualMask; };
    bool IsCalibratedVisualMaskValueSet() const { return !CalibratedVisualMask.IsEmpty(); };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Randomly generates a new visual mask, does not guarantee uniqueness
    FString GenerateNewRandomVisualMask() const;

private:
    // Set references
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

    // Specific imports from tag, true if new value is written
    bool ImportVisualMaskValue(bool bOverwrite = false);
    bool ImportCalibratedVisualMaskValue(bool bOverwrite = false);

    // Apply color to the dynamic material
    bool ApplyVisualMaskColorToDynamicMaterial();

    // Check if the dynmic material is valid
    bool HasValidDynamicMaterial() const;

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
    UMaterial* VisualMaskMaterial;

    // Dynamic material used for setting various mask colors
    UPROPERTY()
    UMaterialInstanceDynamic* VisualMaskDynamicMaterial;

    // Cached original materials
    UPROPERTY()
    TArray<UMaterialInterface*> OriginalMaterials;
};
