// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/SLBaseIndividual.h"
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

    // Get the child individual (Constraint1)
    USLBaseIndividual* GetChildIndividual() const { return ChildIndividual; };

    // Get the parent individual (Constraint2)
    USLBaseIndividual* GetParentIndividual() const { return ParentIndividual; };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Check if the constraint instance is valid
    bool HasValidConstraintInstance() const;

    // Set constraint instance
    virtual bool SetConstraintInstance() { return false; }

    // Check if the individuals are valid
    bool HasValidChildIndividual() const;

    // Set the child individual object
    virtual bool SetChildIndividual() { return false; };

    // Check if the parent individual is valid
    bool HasValidParentIndividual() const;

    // Set the parent individual object
    virtual bool SetParentIndividual() { return false; };

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
    USLBaseIndividual* ChildIndividual;

    // Constraint2 - 'parent' bone in a PhysicsAsset
    UPROPERTY(VisibleAnywhere, Category = "SL")
    USLBaseIndividual* ParentIndividual;

    // Physics representation of the constraint
    // only UObjects support UPROPERTY, the constraint instance is thus not persistent, it should be set every time on PostLoad()
    FConstraintInstance* ConstraintInstance;

    /*UPROPERTY(EditAnywhere, Category = "SL")
    FConstraintInstance ConstraintInstance;*/
};
