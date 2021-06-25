// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "SLLandscapeIndividual.generated.h"

// Forward declarations
class ALandscape;

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLLandscapeIndividual : public USLVisibleIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLLandscapeIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("LandscapeIndividual"); };

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

    // Check if the landscape actor is set
    bool HasValidLandscapeActor() const;

    // Set the static mesh component
    bool SetLandscapeActor();

private:
    // The landscape actor
    UPROPERTY()
    ALandscape* LandscapeActor;
};
