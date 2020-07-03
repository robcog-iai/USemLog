// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLPerceivableIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLBoneIndividual.generated.h"

/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLBoneIndividual : public USLPerceivableIndividual
{
    GENERATED_BODY()

friend class USLSkeletalIndividual;

public:
    // Ctor
    USLBoneIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
    virtual void PostInitProperties() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImport = false);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("BoneIndividual"); };

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

    // Set pointer to parent actor
    virtual bool SetParentActor() override;

    //// Check if the skeletal parent actor and individual is set
    //bool  HasValidSekeletalParent() const 
    //{
    //    return ParentSkeletalActor && ParentSkeletalActor->IsValidLowLevel() && !ParentSkeletalActor->IsPendingKill()
    //        && ParentSkeletalIndividual && ParentSkeletalIndividual->IsValidLowLevel() && !ParentSkeletalIndividual->IsPendingKill();
    //}

    //// Set pointer to parent skeletal actor
    //bool SetSkeletalParentActor();

    //// Set pointer to parent skeletal individual
    //bool SetSkeletalParentIndividual();

private:
    // States implementations, set references and data
    bool InitImpl();
    bool LoadImpl(bool bTryImport = true);

//protected:
//    // Persistent pointer to the parent skeletal actor
//    UPROPERTY()
//    ASkeletalMeshActor* ParentSkeletalActor;
//
//    // Persistent pointer to the parent skeletal individual
//    UPROPERTY()
//    USLSkeletalIndividual* ParentSkeletalIndividual;
};