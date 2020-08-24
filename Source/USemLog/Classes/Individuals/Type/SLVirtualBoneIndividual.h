// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "SLVirtualBoneIndividual.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLVirtualBoneIndividual : public USLBaseIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLVirtualBoneIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;
    
    // Set the parameters required when initalizing the individual
    bool PreInit(int32 NewBoneIndex, bool bReset);

    // Check if the individual is pre initalized
    bool IsPreInit() const { return bIsPreInit; };

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get the bone index
    int32 GetBoneIndex() const { return BoneIndex; };

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("VirtualBoneIndividual"); };

    // Calculate and cache the individuals transform (returns true on a new value)
    virtual bool CalcAndCacheTransform(float Tolerance = 0.25f, FTransform* OutTransform = nullptr) override;

    // Get the attachment location name (bone/socket)
    FName GetAttachmentLocationName();

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Set pointer to parent actor
    virtual bool SetParentActor() override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport = true);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

    // Check if the bone index is valid
    bool HasValidBoneIndex() const;

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

    // Cached transform
    FTransform CachedTransform;
};
