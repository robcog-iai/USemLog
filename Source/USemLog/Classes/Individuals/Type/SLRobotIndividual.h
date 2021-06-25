// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "SLRobotIndividual.generated.h"


/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLRobotIndividual : public USLVisibleIndividual
{
    GENERATED_BODY()

public:
    // Ctor
    USLRobotIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references 
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get all children of the individual in a newly created array
    const TArray<USLBaseIndividual*> GetChildrenIndividuals() const override;

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("RobotIndividual"); };

    /* Begin Visible individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bIncludeChildren = false) override;

    // Apply original materials
    virtual bool ApplyOriginalMaterials() override;
    /* End Visible individual interface */

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

};
