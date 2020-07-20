// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLBaseIndividual.h"
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

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("VirtualBoneIndividual"); };

    // Calculate the current bone transform
    bool CacheCurrentBoneTransform();

    // Get the cached bone transform
    FTransform GetCachedTransform() const { return CachedTransform; };

    // Get the attachment location name (bone/socket)
    FName GetAttachmentLocationName();

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() const override;

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
    bool HasValidSkeletalMesh() const;

    // Set sekeletal mesh
    bool SetSkeletalMesh();

protected:    
    // Pre init
    UPROPERTY(VisibleAnywhere, Category = "SL")
    bool bIsPreInit;

    // Bone index
    UPROPERTY(VisibleAnywhere, Category = "SL")
    int32 BoneIndex;

    // Parent skeletal mesh
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Cached transform
    FTransform CachedTransform;
};
