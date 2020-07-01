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
UCLASS(ClassGroup = SL)
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

    // Get the constraint actors
    AActor* GetConstraintActor1() const { return ConstraintActor1; };
    AActor* GetConstraintActor2() const { return ConstraintActor2; };
    
    // Get the constraint individuals
    USLBaseIndividual* GetConstraintIndividual1() const { return ConstraintIndividual1; };
    USLBaseIndividual* GetConstraintIndividual2() const { return ConstraintIndividual2; };

    // Check if the constraint actors are valid
    bool HasValidConstraintActor1() const { return ConstraintActor1 && ConstraintActor1->IsValidLowLevel() && !ConstraintActor1->IsPendingKill(); };
    bool HasValidConstraintActor2() const { return ConstraintActor2 && ConstraintActor2->IsValidLowLevel() && !ConstraintActor2->IsPendingKill(); };
    bool HasValidConstraintActors() const { return HasValidConstraintActor1() && HasValidConstraintActor2(); };

    // Check if the individuals are valid
    bool HasValidConstraintIndividual1() const { return ConstraintIndividual1 && ConstraintIndividual1->IsValidLowLevel() && !ConstraintIndividual1->IsPendingKill(); };
    bool HasValidConstraintIndividual2() const { return ConstraintIndividual2 && ConstraintIndividual2->IsValidLowLevel() && !ConstraintIndividual2->IsPendingKill(); };
    bool HasValidConstraintIndividuals() const { return HasValidConstraintIndividual1() && HasValidConstraintIndividual2(); };

    // Check if all constraint entities are valid
    bool HasValidConstaintEntities() const { return HasValidConstraintActors() && HasValidConstraintIndividuals(); };

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
