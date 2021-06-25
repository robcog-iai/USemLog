// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLBaseConstraintIndividual.h"
#include "SLBoneConstraintIndividual.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLBoneConstraintIndividual : public USLBaseConstraintIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLBoneConstraintIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Set the parameters required when initalizing the individual
    bool PreInit(int32 NewConstraintIndex, bool bReset);

    // Check if the individual is pre initalized
    bool IsPreInit() const { return bIsPreInit; };

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Calculate and cache the individuals transform (returns true on a new value)
    virtual bool UpdateCachedPose(float Tolerance = 0.25f, FTransform* OutPose = nullptr) override;

    // Get the constraint index
    int32 GetConstraintIndex() const { return ConstraintIndex; };

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("BoneConstraintIndividual"); };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Set pointer to parent actor
    virtual bool SetParentActor() override;
    
    // Set the child individual object
    virtual bool SetConstraint1Individual() override;

    // Set the parent individual object
    virtual bool SetConstraint2Individual() override;

    // Get the constraint instance of the individual
    virtual FConstraintInstance* GetConstraintInstance() const override;

    // Check if the constraint index is valid
    bool HasValidConstraintIndex() const;

    // Check if the static mesh component is set
    bool HasValidSkeletalMeshComponent() const;

    // Set the skeletal mesh component
    bool SetSkeletalMeshComponent();


private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

protected:
    // Needs to be set in order to be initialized
    UPROPERTY(VisibleAnywhere, Category = "SL")
    bool bIsPreInit;

    // Position of this constraint within the array in the SkeletalMeshComponent
    UPROPERTY(VisibleAnywhere, Category = "SL")
    int32 ConstraintIndex;

    // Parent skeletal mesh
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;
};
