// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLBaseIndividual.h"
#include "SLConstraintIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USEMLOG_API USLConstraintIndividual : public USLBaseIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLConstraintIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
    virtual void PostInitProperties() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false, bool bTryImportFromTags = false);

    // Save data to owners tag
    virtual bool ExportToTag(bool bOverwrite = false) override;

    // Load data from owners tag
    virtual bool ImportFromTag(bool bOverwrite = false) override;

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("ConstraintIndividual"); };

protected:
    // Clear all values of the individual
    virtual void InitReset() override;

    // Clear all data of the individual
    virtual void LoadReset() override;

    // Clear any bound delegates (called when init is reset)
    virtual void ClearDelegateBounds() override;

private:
    // States implementations, set references and data
    bool InitImpl();
    bool LoadImpl(bool bTryImportFromTags = true);

protected:
    // First constraint actor
    UPROPERTY(VisibleAnywhere, Category = "SL")
    AActor* ConstraintActor1;

    // First constraint individual
    UPROPERTY(VisibleAnywhere, Category = "SL")
    USLBaseIndividual* ConstraintIndividual1;

    // Second constraint actor
    UPROPERTY(VisibleAnywhere, Category = "SL")
    AActor* ConstraintActor2;

    // Second constraint individual
    UPROPERTY(VisibleAnywhere, Category = "SL")
    USLBaseIndividual* ConstraintIndividual2;

};
