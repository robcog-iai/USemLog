// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLPerceivableIndividual.h"
#include "SLRigidIndividual.generated.h"

// Forward declarations
class UStaticMeshComponent;

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLRigidIndividual : public USLPerceivableIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLRigidIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
    virtual void PostInitProperties() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImport = false);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("RigidIndividual"); };

    /* Begin Perceivable individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bPrioritizeChildren = false) override;

    // Apply original materials
    virtual bool ApplyOriginalMaterials() override;
    /* End Perceivable individual interface */

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() const override;

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
    bool HasValidStaticMesh() const;

    // Set the static mesh component
    bool SetStaticMesh();

private:
    // The visual component of the owner
    UPROPERTY()
    UStaticMeshComponent* StaticMeshComponent;
};
