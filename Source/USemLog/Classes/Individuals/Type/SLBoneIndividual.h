// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "SLBoneIndividual.generated.h"

// Forward declaration
class USkeletalMeshComponent;
class UPoseableMeshComponent;

/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLBoneIndividual : public USLVisibleIndividual
{
    GENERATED_BODY()

    // TODO see if this makes sense for setting the class, ids etc.
    //// Only the owning skeletal individual should be able to change values
    //friend class USLSkeletalIndividal;

public:
    // Ctor
    USLBoneIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Set the parameters required when initalizing the individual (should be called right after construction by the skeletal individual)
    bool PreInit(int32 NewBoneIndex, int32 NewMaterialIndex, bool bReset);

    // Check if the individual is pre initalized 
    bool IsPreInit() const { return bIsPreInit; };

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("BoneIndividual"); };

    /* Begin Visible individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bIncludeChildren = false) override;

    // Apply original materials
    virtual bool ApplyOriginalMaterials() override;
    /* End Visible individual interface */

    // Calculate and cache the individuals transform (returns true on a new value)
    virtual bool UpdateCachedPose(float Tolerance = 0.25f, FTransform* OutPose = nullptr);

    // Get the attachment location name (bone/socket)
    FName GetAttachmentLocationName();

    // Get the bone index
    int32 GetBoneIndex() const { return BoneIndex; };

    // Get the material index represented by the bone
    int32 GetMaterialIndex() const { return MaterialIndex; };

    // Get the skeletal mesh component the bone is used in
    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; };

    // Get the poseable mesh component (if available)
    UPoseableMeshComponent* GetPoseableMeshComponent();

    // Return the curently active (visible) mesh compoent
    UMeshComponent* GetVisibleMeshComponent();

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Set pointer to parent actor
    virtual bool SetParentActor() override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

    // Check if the bone index is valid
    bool HasValidBoneIndex() const;

    // Check if the material index is valid
    bool HasValidMaterialIndex() const;

    // Check if the static mesh component is set
    bool HasValidSkeletalMeshComponent() const;

    // Set sekeletal mesh
    bool SetSkeletalMeshComponent();

    // Check if a parent individual is set
    bool HasValidParentIndividual() const;

    // Set parent individual (if any) it might be root bone
    bool SetParentIndividual();

    // Check if any children individuals are set
    bool HasValidChildrenIndividuals() const;

    // Set child individual (if any) it might be a leaf bone
    bool SetChildrenIndividuals();

    // Clear children individual
    void ClearChildrenIndividuals();

protected:
    // Pre init
    UPROPERTY(VisibleAnywhere, Category = "SL")
    bool bIsPreInit;

    // Mask material index
    UPROPERTY(VisibleAnywhere, Category = "SL")
    int32 MaterialIndex;

    // Bone index
    UPROPERTY(VisibleAnywhere, Category = "SL")
    int32 BoneIndex;

    // Parent individual (can be nullptr if the bone individual is the root)
    UPROPERTY(VisibleAnywhere, Category = "SL")
    USLBaseIndividual* ParentIndividual;

    // Children individual (can be empty if the bone individual is a leaf)
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLBaseIndividual*> ChildrenIndividuals;

    // Parent skeletal mesh
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Parent poseable mesh component (not persistent)
    UPoseableMeshComponent* PoseableMeshComponent;
};
