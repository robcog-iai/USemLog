// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLSkeletalIndividual.h"
#include "Individuals/Type/SLBoneIndividual.h"
#include "Individuals/Type/SLVirtualBoneIndividual.h"
#include "Individuals/Type/SLBoneConstraintIndividual.h"
#include "Individuals/SLIndividualComponent.h"
#include "Skeletal/SLSkeletalDataAsset.h"

#include "Materials/MaterialInstanceDynamic.h"

#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/ConstraintInstance.h"

// Ctor
USLSkeletalIndividual::USLSkeletalIndividual()
{
	//bIsConnected = false;
	SkeletalMeshComponent = nullptr;
	SkeletalDataAsset = nullptr;
	PoseableMeshComponent = nullptr;
}

//// Do any object-specific cleanup required immediately after loading an object.
//void USLSkeletalIndividual::PostLoad()
//{
//	Super::PostLoad();
//	if (!IsConnected())
//	{
//		Connect();
//	}
//}

// Called before destroying the object.
void USLSkeletalIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLSkeletalIndividual::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init(bReset) && InitImpl());
	return IsInit();
}

// Load semantic data
bool USLSkeletalIndividual::Load(bool bReset, bool bTryImport)
{
	if (bReset)
	{
		LoadReset();
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			return false;
		}
	}

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}

//// Listen to the children individual object delegates
//bool USLSkeletalIndividual::Connect()
//{
//	if (IsConnected())
//	{
//		return true;
//	}
//	SetIsConnected(BindChildrenDelegates());
//	return IsConnected();
//}

//// Save values externally
//bool USLSkeletalIndividual::ExportValues(bool bOverwrite)
//{
//	bool RetVal = Super::ExportValues();
//	for (const auto& CI : GetChildrenIndividuals())
//	{
//		if (CI->ExportValues(bOverwrite))
//		{
//			RetVal = true;
//		}
//	}
//	return RetVal;
//}
//
//// Load values externally
//bool USLSkeletalIndividual::ImportValues(bool bOverwrite)
//{
//	bool RetVal = Super::ImportValues();
//	for (const auto& CI : GetChildrenIndividuals())
//	{
//		if (CI->ImportValues(bOverwrite))
//		{
//			RetVal = true;
//		}
//	}
//	return RetVal;
//}

//// Clear exported values
//bool USLSkeletalIndividual::ClearExportedValues()
//{
//	bool RetVal = Super::ClearExportedValues();
//	for (const auto& CI : GetChildrenIndividuals())
//	{
//		if (CI->ClearExportedValues())
//		{
//			RetVal = true;
//		}
//	}
//	return false;
//}

// Get all children of the individual in a newly created array
const TArray<USLBaseIndividual*> USLSkeletalIndividual::GetChildrenIndividuals() const
{
	TArray<USLBaseIndividual*> Children;
	for (const auto& BI : BoneIndividuals)
	{
		Children.Add(BI);
	}	
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		Children.Add(VBI);
	}
	for (const auto& BCI : BoneConstraintIndividuals)
	{
		Children.Add(BCI);
	}
	return Children;
}

// Check if child can be attached, if so return its location bone/socket name)
bool USLSkeletalIndividual::IsChildAttachable(USLBaseIndividual* Child, FName& OutName)
{
	if (USLBoneIndividual* AsBI = Cast<USLBoneIndividual>(Child))
	{
		if (BoneIndividuals.Contains(AsBI))
		{
			OutName = AsBI->GetAttachmentLocationName();
			return true;
		}
	}
	else if (USLVirtualBoneIndividual* AsVBI = Cast<USLVirtualBoneIndividual>(Child))
	{
		if (VirtualBoneIndividuals.Contains(AsVBI))
		{
			OutName = AsVBI->GetAttachmentLocationName();
			return true;
		}
	}
	return false;
}

// Search and return the bone (virtual or non-virtual) with the given index as a base individual (nullptr if not found)
USLBaseIndividual* USLSkeletalIndividual::GetBoneIndividual(int32 Index) const
{
	if (auto FoundBI = BoneIndividuals.FindByPredicate(
		[Index](const USLBoneIndividual* InItem) { return InItem->GetBoneIndex() == Index; }))
	{
		return *FoundBI;
	}
	else if (auto FoundVBI = VirtualBoneIndividuals.FindByPredicate(
		[Index](const USLVirtualBoneIndividual* InItem) { return InItem->GetBoneIndex() == Index; }))
	{
		return *FoundVBI;
	}
	return nullptr;
}

// Apply visual mask material
bool USLSkeletalIndividual::ApplyMaskMaterials(bool bIncludeChildren)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		if (bIncludeChildren)
		{
			for (const auto& BI : BoneIndividuals)
			{
				BI->ApplyMaskMaterials();
			}
		}
		else
		{
			for (int32 MatIdx = 0; MatIdx < SkeletalMeshComponent->GetNumMaterials(); ++MatIdx)
			{
				SkeletalMeshComponent->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
			}

			// Apply for poseable mesh clone if there is one
			if (UPoseableMeshComponent* PMC = GetPoseableMeshComponent())
			{
				for (int32 MatIdx = 0; MatIdx < PMC->GetNumMaterials(); ++MatIdx)
				{
					PMC->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
				}
			}
		}

		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLSkeletalIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
		int32 MatIdx = 0;
		for (const auto& Mat : OriginalMaterials)
		{
			SkeletalMeshComponent->SetMaterial(MatIdx, Mat);
			++MatIdx;
		}

		// Apply for poseable mesh clone if there is one
		if (UPoseableMeshComponent* PMC = GetPoseableMeshComponent())
		{
			int32 PMCMatIdx = 0;
			for (const auto& Mat : OriginalMaterials)
			{
				PMC->SetMaterial(PMCMatIdx, Mat);
				++PMCMatIdx;
			}
		}

		bIsMaskMaterialOn = false;
				
		// Bones share the same original materials with the skeletal parent, this will only set the flag value
		for (const auto& BI : BoneIndividuals)
		{
			BI->ApplyOriginalMaterials();
		}

		return true;
	}
	return false;
}

// Get the poseable mesh component (if available)
UPoseableMeshComponent* USLSkeletalIndividual::GetPoseableMeshComponent()
{
	if (PoseableMeshComponent)
	{
		return PoseableMeshComponent;
	}
	if (UActorComponent* AC = GetParentActor()->GetComponentByClass(UPoseableMeshComponent::StaticClass()))
	{
		PoseableMeshComponent = CastChecked<UPoseableMeshComponent>(AC);
		return PoseableMeshComponent;
	}
	return nullptr;
}

// Return the curently active (visible) mesh compoent
UMeshComponent* USLSkeletalIndividual::GetVisibleMeshComponent()
{
	if (SkeletalMeshComponent && SkeletalMeshComponent->IsVisible())
	{
		return SkeletalMeshComponent;
	}
	return GetPoseableMeshComponent();
}

// Get class name, virtual since each invidiual type will have different name
FString USLSkeletalIndividual::CalcDefaultClassValue()
{
	if (HasValidParentActor() || SetParentActor())
	{
		if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
		{
			FString ClassName = SkeletalMeshComponent->SkeletalMesh->GetFullName();
			int32 FindCharPos;
			ClassName.FindLastChar('.', FindCharPos);
			ClassName.RemoveAt(0, FindCharPos + 1);
			ClassName.RemoveFromStart(TEXT("SK_"));
			return ClassName;
		}
	}
	return GetTypeName();
}

//// Set the connected flag, broadcast on new value
//void USLSkeletalIndividual::SetIsConnected(bool bNewValue, bool bBroadcast)
//{
//	if (bIsConnected != bNewValue)
//	{
//		bIsConnected = bNewValue;
//		if (bBroadcast)
//		{
//			//OnConnectedChanged.Broadcast(this, bNewValue);
//		}
//	}
//}

// Private init implementation
bool USLSkeletalIndividual::InitImpl()
{
	// Make sure the visual mesh is set
	if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
	{
		// Make sure there is a skeletal semantic data component
		if (HasValidSkeletalDataAsset() || SetSkeletalDataAsset())
		{
			// Make sure the bone individuals are created
			if (HasValidChildrenIndividuals() || CreateChildrenIndividuals())
			{
				//Connect();
				return InitChildrenIndividuals();
			}
		}
	}
	return false;
}

// Private load implementation
bool USLSkeletalIndividual::LoadImpl(bool bTryImport)
{
	if (HasValidChildrenIndividuals())
	{
		return LoadChildrenIndividuals();
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s's bones should be valid here, this should not happen.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Clear all values of the individual
void USLSkeletalIndividual::InitReset()
{
	LoadReset();
	DestroyChildrenIndividuals();
	SkeletalMeshComponent = nullptr;
	SetIsInit(false);
}

// Clear all data of the individual
void USLSkeletalIndividual::LoadReset()
{
	LoadResetChildrenIndividuals();
	SetIsLoaded(false);
}

// Check if the static mesh component is set
bool USLSkeletalIndividual::HasValidSkeletalMeshComponent() const
{
	return SkeletalMeshComponent && SkeletalMeshComponent->IsValidLowLevel() && !SkeletalMeshComponent->IsPendingKill();
}

// Set sekeletal mesh
bool USLSkeletalIndividual::SetSkeletalMeshComponent()
{
	if (HasValidParentActor() || SetParentActor())
	{
		if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(ParentActor))
		{
			if (USkeletalMeshComponent* SMC = SkMA->GetSkeletalMeshComponent())
			{
				SkeletalMeshComponent = SMC;
				OriginalMaterials = SMC->GetMaterials();

				//// DEBUG LOGs
				//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's Constraints:"), *FString(__FUNCTION__), __LINE__, *GetFullName());
				//for (const auto CI : SkeletalMeshComponent->Constraints)
				//{					
				//	UE_LOG(LogTemp, Warning, TEXT("\t\t\t JointName=%s; ConstraintIndex=%ld;"),
				//		*CI->JointName.ToString(), CI->ConstraintIndex);
				//}

				//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's Bodies:"), *FString(__FUNCTION__), __LINE__, *GetFullName());
				//for (const auto BI : SkeletalMeshComponent->Bodies)
				//{					
				//	UE_LOG(LogTemp, Warning, TEXT("\t\t\t GetBodyDebugName=%s; InstanceBodyIndex=%ld; InstanceBodyIndex=%ld;"),
				//		*BI->GetBodyDebugName(), BI->InstanceBodyIndex, BI->InstanceBoneIndex);
				//}
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ParentActor has no SkeletalMeshComponent, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s ParentActor is not a SkeletalMeshActor, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s ParentActor is not set, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Check if skeleltal bone description asset is available
bool USLSkeletalIndividual::HasValidSkeletalDataAsset() const
{
	return SkeletalDataAsset && SkeletalDataAsset->IsValidLowLevel() && !SkeletalDataAsset->IsPendingKill();
}

// Set the skeletal data component from the individual component
bool USLSkeletalIndividual::SetSkeletalDataAsset()
{
	// Outer should be the individual component
	if (USLIndividualComponent* IC = Cast<USLIndividualComponent>(GetOuter()))
	{
		if (UDataAsset** DA = IC->OptionalDataAssets.Find(USLIndividualComponent::SkelDataAssetKey))
		{
			if (USLSkeletalDataAsset* SkDA = Cast<USLSkeletalDataAsset>(*DA))
			{
				SkeletalDataAsset = SkDA;
				return true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's has no skeletal data asset.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's outer is not an individual component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Check if all the bones are valid
bool USLSkeletalIndividual::HasValidChildrenIndividuals() const
{
	// Bones are not valid without a skeletal mesh set
	if (!HasValidSkeletalMeshComponent())
	{
		return false;
	}

	// Check if the number of bones and constraints are in sync with the skeletal mesh
	if ((BoneIndividuals.Num() + VirtualBoneIndividuals.Num()) != SkeletalMeshComponent->GetNumBones())
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d %s's number of bones (BonesNum=%ld + VirtualBonesNum=%ld) is out of sync with the skeletal mesh component BonesNum=%ld.."),
		//	*FString(__FUNCTION__), __LINE__, *GetFullName(), BoneIndividuals.Num(), VirtualBoneIndividuals.Num(), SkeletalMeshComponent->GetNumBones());
		return false;
	}

	if (BoneConstraintIndividuals.Num() != SkeletalMeshComponent->Constraints.Num())
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d %s's number of constraints bone %ld is out of sync with the skeletal mesh component constraints %ld.."),
		//	*FString(__FUNCTION__), __LINE__, *GetFullName(), BoneConstraintIndividuals.Num(), SkeletalMeshComponent->Constraints.Num());
		return false;
	}

	// Make sure all children are valid and pre initalized
	bool bAllChildrenAreValid = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsValidLowLevel() || BI->IsPendingKill() || !BI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child individual %s is not valid.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *BI->GetFullName());
			bAllChildrenAreValid = false;
		}
	}
	for (const auto VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsValidLowLevel() || VBI->IsPendingKill() || !VBI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child individual %s is not valid.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *VBI->GetFullName());
			bAllChildrenAreValid = false;
		}
	}
	for (const auto& BCI : BoneConstraintIndividuals)
	{
		if (!BCI->IsValidLowLevel() || BCI->IsPendingKill() || !BCI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child individual %s is not valid.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *BCI->GetFullName());
			bAllChildrenAreValid = false;
		}
	}
	return bAllChildrenAreValid;
}

// Create new bone objects
bool USLSkeletalIndividual::CreateChildrenIndividuals()
{
	if (HasValidChildrenIndividuals())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s already has valid children individuals, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	// Destroy any previously created individuals
	if (GetChildrenIndividuals().Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has children individuals, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		DestroyChildrenIndividuals();
	}


	if (!HasValidSkeletalMeshComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's skeletal mesh is not set, cannot create bone individuals.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	// Set the bone children using data from the skeletal data asset
	for (const auto& BoneDataKV : SkeletalDataAsset->BoneIndexClass)
	{
		const FString BoneClassName = BoneDataKV.Value;
		int32 BoneIndex = BoneDataKV.Key;

		if (!BoneClassName.IsEmpty())
		{
			USLBoneIndividual* BI = NewObject<USLBoneIndividual>(this);
			int32 MatIdx = SkeletalMeshComponent->GetMaterialIndex(FName(*BoneClassName));
			if (MatIdx == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's a bone will be invalid, no material slot found with the name: %s .."),
					*FString(__FUNCTION__), __LINE__, *GetFullName(), *BoneDataKV.Value);
			}
			BI->PreInit(BoneIndex, MatIdx, false);
			BoneIndividuals.Add(BI);
		}
		else
		{
			USLVirtualBoneIndividual* VBI = NewObject<USLVirtualBoneIndividual>(this);
			VBI->PreInit(BoneIndex, false);
			VirtualBoneIndividuals.Add(VBI);
		}
	}

	// Set the skeletal bone constraints
	for (const auto& CI : SkeletalMeshComponent->Constraints)
	{
		USLBoneConstraintIndividual* BCI = NewObject<USLBoneConstraintIndividual>(this);
		BCI->PreInit(CI->ConstraintIndex, false);
		BoneConstraintIndividuals.Add(BCI);
	}

	return HasValidChildrenIndividuals();
}

// Call init on all bones, true if all succesfully init
bool USLSkeletalIndividual::InitChildrenIndividuals()
{
	bool bAllChildrenAreInit = true;
	for (const auto& CI : GetChildrenIndividuals())
	{
		if (!(CI->IsInit() || CI->Init(false)))
		{
			bAllChildrenAreInit = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child %s could not be init.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *CI->GetFullName());
		}
	}
	return bAllChildrenAreInit;
}

// Call load on all bones, true if all succesfully loaded
bool USLSkeletalIndividual::LoadChildrenIndividuals()
{
	bool bAllChildrenAreLoaded = true;
	for (const auto& CI : GetChildrenIndividuals())
	{
		if (!(CI->IsLoaded() || CI->Load(false, true)))
		{
			bAllChildrenAreLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child %s could not be loaded.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *CI->GetFullName());
		}
	}
	return bAllChildrenAreLoaded;
}

// Destroy bone individuals
void USLSkeletalIndividual::DestroyChildrenIndividuals()
{
	for (const auto& CI : GetChildrenIndividuals())
	{
		CI->ConditionalBeginDestroy();
	}
	BoneIndividuals.Empty();
	VirtualBoneIndividuals.Empty();
	BoneConstraintIndividuals.Empty();
}

// Reset child individuals
void USLSkeletalIndividual::LoadResetChildrenIndividuals()
{
	for (const auto& CI : GetChildrenIndividuals())
	{
		CI->Load(true, false);
	}
}

//// Bind to children delegates
//bool USLSkeletalIndividual::BindChildrenDelegates()
//{
//	if (!HasValidChildrenIndividuals())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no valid children, cannot bind to their delegates.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName());
//		return false;
//	}
//
//	for (const auto& BI : BoneIndividuals)
//	{
//		BindChildIndividualDelegates(BI);
//	}
//	for (const auto& VBI : VirtualBoneIndividuals)
//	{
//		BindChildIndividualDelegates(VBI);
//	}
//
//	return true;
//}
//
//// Listen to children changes
//void USLSkeletalIndividual::BindChildIndividualDelegates(USLBaseIndividual* ChildrenIndividuals)
//{
//	// Bind init change delegate
//	if (!ChildrenIndividuals->OnInitChanged.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildInitChange))
//	{
//		ChildrenIndividuals->OnInitChanged.AddDynamic(this, &USLSkeletalIndividual::OnChildInitChange);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on init delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildrenIndividuals->GetFullName());
//	}
//
//	// Bind load change delegate
//	if (!ChildrenIndividuals->OnLoadedChanged.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildLoadedChange))
//	{
//		ChildrenIndividuals->OnLoadedChanged.AddDynamic(this, &USLSkeletalIndividual::OnChildLoadedChange);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on loaded delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildrenIndividuals->GetFullName());
//	}
//
//	// Bind delegeates cleared 
//	if (!ChildrenIndividuals->OnDelegatesCleared.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildDelegatesCleared))
//	{
//		ChildrenIndividuals->OnDelegatesCleared.AddDynamic(this, &USLSkeletalIndividual::OnChildDelegatesCleared);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on delegates cleared delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildrenIndividuals->GetFullName());
//	}
//
//	/* Values delegates */
//	if (!ChildrenIndividuals->OnNewValue.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildNewValue))
//	{
//		ChildrenIndividuals->OnNewValue.AddDynamic(this, &USLSkeletalIndividual::OnChildNewValue);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on delegates cleared delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildrenIndividuals->GetFullName());
//	}
//}
//
//// Triggered on child individual init flag change
//void USLSkeletalIndividual::OnChildInitChange(USLBaseIndividual* Individual, bool bNewValue)
//{
//	if (IsInit() && !bNewValue)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child %s is not init anymore, this should not happen while the parent is still init.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *Individual->GetFullName());
//		// Child is not init anymore, parent should not be either
//		SetIsInit(false);
//	}
//}
//
//// Triggered on child individual loaded flag change
//void USLSkeletalIndividual::OnChildLoadedChange(USLBaseIndividual* Individual, bool bNewValue)
//{
//	if (IsLoaded() && !bNewValue)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child %s is not loaded anymore, this should not happen while the parent is still loaded.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *Individual->GetFullName());
//		// Child is not loaded anymore, parent should not be either
//		SetIsLoaded(false);
//	}
//}
//
//// Triggered when a child individual value is changed
//void USLSkeletalIndividual::OnChildNewValue(USLBaseIndividual* Individual, const FString& Key, const FString& NewValue)
//{
//	// TODO
//	//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s has a new value [%s:%s].."),
//	//	*FString(__FUNCTION__), __LINE__, *GetFullName(), *Individual->GetFullName(), *Key, *NewValue);
//}
//
//// Triggered when a child individual delegates are cleared (triuggered when InitReset is called)
//void USLSkeletalIndividual::OnChildDelegatesCleared(USLBaseIndividual* Individual)
//{
//	if (IsInit())
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child %s's delegates are cleared, this should not happen while the parent is init.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *Individual->GetFullName());
//		// Child is not init anymore, parent should not be either
//		SetIsInit(false);
//	}
//}
