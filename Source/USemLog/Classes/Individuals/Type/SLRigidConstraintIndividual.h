// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLBaseConstraintIndividual.h"
#include "SLRigidConstraintIndividual.generated.h"

// Forward declarations
class AActor;
class UPhysicsConstraintComponent;

/**
 * 
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLRigidConstraintIndividual : public USLBaseConstraintIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLRigidConstraintIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("RigidConstraintIndividual"); };

    // Get constraint actor1 (can be considered as 'child' if using the skeletal mesh rules)
    AActor* GetConstraintActor1() const { return ConstraintActor1; };

    // Get the constraint actor2 (can be considered as 'parent' if using skeletal mesh rules)
    AActor* GetConstraintActor2() const { return ConstraintActor2; };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    // Set the child individual object
    virtual bool SetConstraint1Individual() override;

    // Set the parent individual object
    virtual bool SetConstraint2Individual() override;

    // Get the constraint instance of the individual
    virtual FConstraintInstance* GetConstraintInstance() const override;

    // Check if the constraint component is valid
    bool HasValidConstraintComponent() const;

    // Set the constraint component
    bool SetConstraintComponent();

    // Check if the constraint actor1 (child) is valid
    bool HasValidConstraintActor1() const;

    // Set constraint actor1 (child)
    bool SetConstraintActor1();

    // Check if the constraint actor2 (parent) is valid
    bool HasValidConstraintActor2() const;

    // Set constraint actor2 (parent)
    bool SetConstraintActor2();

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
    // First constraint actor
    UPROPERTY(VisibleAnywhere, Category = "SL")
    AActor* ConstraintActor1;

    // Second constraint actor
    UPROPERTY(VisibleAnywhere, Category = "SL")
    AActor* ConstraintActor2;

    // Constraint component
    UPROPERTY(VisibleAnywhere, Category = "SL")
    UPhysicsConstraintComponent* ConstraintComponent;
};
