// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLBaseIndividual.h"
#include "SLLightIndividual.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLLightIndividual : public USLBaseIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLLightIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImport = false);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("LightIndividual"); };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() const override;

    // Clear all values of the individual
    virtual void InitReset() override;

    // Clear all data of the individual
    virtual void LoadReset() override;

    // Clear any bound delegates (called when init is reset)
    virtual void ClearDelegates() override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport = true);
};
