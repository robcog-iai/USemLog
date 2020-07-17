// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLPerceivableIndividual.h"
#include "SLSkeletalIndividual.generated.h"

// Forward declaration
class USLBoneIndividual;
class USLVirtualBoneIndividual;
class USkeletalMeshComponent;
class USLSkeletalDataAsset;
class USLSkeletalDataComponent;

/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLSkeletalIndividual : public USLPerceivableIndividual
{
    GENERATED_BODY()

    // Give bones access to the private members
    friend class USLBoneIndividual;
    friend class USLVirtualBoneIndividual;

public:
    // Ctor
    USLSkeletalIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references 
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Save values externally
    virtual bool ExportValues(bool bOverwrite = false) override;

    // Load values externally
    virtual bool ImportValues(bool bOverwrite = false) override;

    // Clear exported values
    virtual bool ClearExportedValues() override;

    // Return a const version of the bone individuals
    const TArray<USLBoneIndividual*>& GetBoneIndividuals() const { return BoneIndividuals; };

    // Return a const version of the virtual bones
    const TArray<USLVirtualBoneIndividual*>& GetVirtualBoneIndividuals() const { return VirtualBoneIndividuals; };

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("SkeletalIndividual"); };

    /* Begin Perceivable individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bIncludeChildren = false) override;

    // Apply original materials
    virtual bool ApplyOriginalMaterials() override;
    /* End Perceivable individual interface */

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() const override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

    // Check if the static mesh component is set
    bool HasValidSkeletalMesh() const;

    // Set sekeletal mesh
    bool SetSkeletalMesh();

    // Check if skeleltal bone description asset is set
    bool HasValidSkeletalDataAsset() const;

    // Set the skeletal bone description data asset
    bool SetSkeletalDataAsset();

    // Check if all the bones are valid
    bool HasValidBones() const;

    // Create the bones individuals
    bool CreateBoneIndividuals();

    // Call init on all bones, true if all succesfully init
    bool InitBoneIndividuals();

    // Call load on all bones, true if all succesfully loaded
    bool LoadBoneIndividuals();

    // Destroy bone individuals
    void DestroyBoneIndividuals();

    // Reset bone individuals
    void ResetBoneIndividuals();

protected:
    // Semantically annotated bones
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLBoneIndividual*> BoneIndividuals;

    // Bones without visual
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLVirtualBoneIndividual*> VirtualBoneIndividuals;

private:
    // The visual component of the owner
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Semantic data of the skeletal component
    UPROPERTY()
    USLSkeletalDataAsset* SkeletalDataAsset;
};
