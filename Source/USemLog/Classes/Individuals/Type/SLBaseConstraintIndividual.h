// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "SLBaseConstraintIndividual.generated.h"

// Forward declarations
struct FConstraintInstance;

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLBaseConstraintIndividual : public USLBaseIndividual
{
    GENERATED_BODY()

public:
    // Ctor
    USLBaseConstraintIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("BaseConstraintIndividual"); };

    // Get constraint1 ('child' bone in a PhysicsAsset) 
    USLBaseIndividual* GetConstraint1Individual() const { return ConstraintIndividual1; };

    // Get constraint2 ('parent' bone in a PhysicsAsset) 
    USLBaseIndividual* GetConstraint2Individual() const { return ConstraintIndividual2; };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Check if the constraint1 individual is valid ('child' bone in a PhysicsAsset) 
    bool HasValidConstraint1Individual() const;

    // Set the constraint1 individual ('child' bone in a PhysicsAsset) 
    virtual bool SetConstraint1Individual() { return false; };

    // Check if the constraint2 individual is valid ('parent' bone in a PhysicsAsset)
    bool HasValidConstraint2Individual() const;

    // Set the constraint2 individual ('parent' bone in a PhysicsAsset) 
    virtual bool SetConstraint2Individual() { return false; };

    // Get the constraint instance of the individual
    virtual FConstraintInstance* GetConstraintInstance() const { return nullptr; }

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
    // Constraint1 - 'child' bone in a PhysicsAsset
    UPROPERTY(VisibleAnywhere, Category = "SL")
    USLBaseIndividual* ConstraintIndividual1;

    // Constraint2 - 'parent' bone in a PhysicsAsset
    UPROPERTY(VisibleAnywhere, Category = "SL")
    USLBaseIndividual* ConstraintIndividual2;

    //// Pointer to the constraint instance
    //FConstraintInstance* ConstraintInstance;
};
