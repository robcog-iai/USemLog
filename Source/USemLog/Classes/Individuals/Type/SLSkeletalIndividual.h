// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "SLSkeletalIndividual.generated.h"

// Forward declaration
class USLBoneIndividual;
class USLVirtualBoneIndividual;
class USLBoneConstraintIndividual;
class USkeletalMeshComponent;
class UPoseableMeshComponent;
class USLSkeletalDataAsset;
class USLSkeletalDataComponent;

/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLSkeletalIndividual : public USLVisibleIndividual
{
    GENERATED_BODY()

    // Give bones access to the private members
    friend class USLBoneIndividual;
    friend class USLVirtualBoneIndividual;

public:
    // Ctor
    USLSkeletalIndividual();

    //// Do any object-specific cleanup required immediately after loading an object. (called only once when loading the map)
    //virtual void PostLoad() override;

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references 
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    //// Listen to the children individual object delegates
    //bool Connect();

    //// True if the component is listening to the individual object delegates (transient)
    //bool IsConnected() const { return bIsConnected; };

    //// Save values externally
    //virtual bool ExportValues(bool bOverwrite = false) override;

    //// Load values externally
    //virtual bool ImportValues(bool bOverwrite = false) override;

    //// Clear exported values
    //virtual bool ClearExportedValues() override;

    // Get all children of the individual in a newly created array
    virtual const TArray<USLBaseIndividual*> GetChildrenIndividuals() const override;

    // Check if child can be attached, if so return its location bone/socket name)
    bool IsChildAttachable(USLBaseIndividual* Child, FName& OutName);

    // Return a const version of the bone individuals
    const TArray<USLBoneIndividual*>& GetBoneIndividuals() const { return BoneIndividuals; };

    // Return a const version of the virtual bones
    const TArray<USLVirtualBoneIndividual*>& GetVirtualBoneIndividuals() const { return VirtualBoneIndividuals; };

    // Return a const version of the bone constraints array
    const TArray<USLBoneConstraintIndividual*>& GetBoneConstraintIndividuals() const { return BoneConstraintIndividuals; };

    // Search and return the bone (virtual or non-virtual) with the given index as a base individual (nullptr if not found)
    USLBaseIndividual* GetBoneIndividual(int32 Index) const;

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("SkeletalIndividual"); };

    /* Begin Visible individual interface */
    // Apply visual mask material
    virtual bool ApplyMaskMaterials(bool bIncludeChildren = false) override;

    // Apply original materials
    virtual bool ApplyOriginalMaterials() override;
    /* End Visible individual interface */

    // Return the skeletal mesh compoent
    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; };

    // Get the poseable mesh component (if available)
    UPoseableMeshComponent* GetPoseableMeshComponent();

    // Return the curently active (visible) mesh compoent
    UMeshComponent* GetVisibleMeshComponent();

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

    //// Set the connected flag, broadcast on new value
    //void SetIsConnected(bool bNewValue, bool bBroadcast = true);

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();

    // Check if the static mesh component is set
    bool HasValidSkeletalMeshComponent() const;

    // Set sekeletal mesh
    bool SetSkeletalMeshComponent();

    // Check if skeleltal bone description asset is set
    bool HasValidSkeletalDataAsset() const;

    // Set the skeletal bone description data asset
    bool SetSkeletalDataAsset();

    // Check if all the bones are valid
    bool HasValidChildrenIndividuals() const;

    // Create the bones individuals
    bool CreateChildrenIndividuals();

    // Call init on all bones, true if all succesfully init
    bool InitChildrenIndividuals();

    // Call load on all bones, true if all succesfully loaded
    bool LoadChildrenIndividuals();

    // Destroy bone individuals
    void DestroyChildrenIndividuals();

    // Reset child individuals
    void LoadResetChildrenIndividuals();

    //// Bind to children delegates
    //bool BindChildrenDelegates();

    //// Listen to children changes
    //void BindChildIndividualDelegates(USLBaseIndividual* ChildrenIndividuals);

    //// Triggered on child individual init flag change
    //UFUNCTION()
    //void OnChildInitChange(USLBaseIndividual* Individual, bool bNewValue);

    //// Triggered on child individual loaded flag change
    //UFUNCTION()
    //void OnChildLoadedChange(USLBaseIndividual* Individual, bool bNewValue);

    //// Triggered when a child individual value is changed
    //UFUNCTION()
    //void OnChildNewValue(USLBaseIndividual* Individual, const FString& Key, const FString& NewValue);

    //// Triggered when a child individual delegates are cleared (triuggered when InitReset is called)
    //UFUNCTION()
    //void OnChildDelegatesCleared(USLBaseIndividual* Individual);

protected:
    // Visible bones
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLBoneIndividual*> BoneIndividuals;

    // Bones without material
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLVirtualBoneIndividual*> VirtualBoneIndividuals;
	
	// Skeletal constraints
    UPROPERTY(VisibleAnywhere, Category = "SL")
    TArray<USLBoneConstraintIndividual*> BoneConstraintIndividuals;

    //// True if the component is listening to the child individual object delegates
    //UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    //uint8 bIsConnected : 1;

private:
    // The visual component of the owner
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Semantic data of the skeletal component
    UPROPERTY()
    USLSkeletalDataAsset* SkeletalDataAsset;

    // Parent poseable mesh component (not persistent)
    UPoseableMeshComponent* PoseableMeshComponent;
};
