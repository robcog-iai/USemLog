// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLPerceivableIndividual.h"
#include "SLSkeletalIndividual.generated.h"

// Forward declaration
class USLBoneIndividual;
class USkeletalMeshComponent;
class USLSkeletalDataComponent;

/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLSkeletalIndividual : public USLPerceivableIndividual
{
    GENERATED_BODY()

public:
    // Ctor
    USLSkeletalIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
    virtual void PostInitProperties() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImport = false);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("SkeletalIndividual"); };

    /* Begin Perceivable individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bPrioritizeChildren = false) override;

    // Apply original materials
    virtual bool ApplyOriginalMaterials() override;
    /* End Perceivable individual interface */

protected:
    // Clear all values of the individual
    virtual void InitReset() override;

    // Clear all data of the individual
    virtual void LoadReset() override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport = true);

    // Check if the static mesh component is set
    bool HasValidSkeletalMesh() const;

    // Set sekeletal mesh
    bool SetSkeletalMesh();

    // Check if skeleltal bone description component is available
    bool HasValidSkeletalDataComponent() const;

    // Set the skeletal data component
    bool SetSkeletalDataComponent();

    // Check if all the bones are valid
    bool HasValidBones() const;

    // Create the bones individuals
    bool CreateBones();

    // Call init on all bones, true if all succesfully init
    bool InitAllBones();

    // Call load on all bones, true if all succesfully loaded
    bool LoadAllBones();

    // Destroy bone individuals
    void DestroyBones();

    // Reset bone individuals
    void ResetBones();

protected:
    // Semantically annotated bones
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLBoneIndividual*> BoneIndividuals;

    // Raw bones
    TArray<FString> RawBones;

private:
    // The visual component of the owner
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Semantic data of the skeletal component
    UPROPERTY()
    USLSkeletalDataComponent* SkelDataComp;
};
