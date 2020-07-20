// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLSkeletalIndividual.h"
#include "Individuals/SLBoneIndividual.h"
#include "Individuals/SLVirtualBoneIndividual.h"
#include "Individuals/SLIndividualComponent.h"
#include "Skeletal/SLSkeletalDataAsset.h"

#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"

#include "Skeletal/SLSkeletalDataComponent.h"

// Ctor
USLSkeletalIndividual::USLSkeletalIndividual()
{
	//bIsConnected = false;
	SkeletalMeshComponent = nullptr;
	SkeletalDataAsset = nullptr;
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
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load component individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
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

// Trigger values as new value broadcast
void USLSkeletalIndividual::TriggerValuesBroadcast()
{
	Super::TriggerValuesBroadcast();

	for (const auto& BI : BoneIndividuals)
	{
		BI->TriggerValuesBroadcast();
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		VBI->TriggerValuesBroadcast();
	}
}

// Save values externally
bool USLSkeletalIndividual::ExportValues(bool bOverwrite)
{
	bool RetVal = Super::ExportValues();
	for (const auto& BI : BoneIndividuals)
	{
		if (BI->ExportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (VBI->ExportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	return RetVal;
}

// Load values externally
bool USLSkeletalIndividual::ImportValues(bool bOverwrite)
{
	bool RetVal = Super::ImportValues();
	for (const auto& BI : BoneIndividuals)
	{
		if (BI->ImportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (VBI->ImportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	return RetVal;
}

// Clear exported values
bool USLSkeletalIndividual::ClearExportedValues()
{
	bool RetVal = Super::ClearExportedValues();
	for (const auto& BI : BoneIndividuals)
	{
		if (BI->ClearExportedValues())
		{
			RetVal = true;
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (VBI->ClearExportedValues())
		{
			RetVal = true;
		}
	}
	return false;
}

// Get all children of the individual
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
		bIsMaskMaterialOn = false;

		// Bones share the same original materials with the skeletal parent
		for (const auto& BI : BoneIndividuals)
		{
			BI->ApplyOriginalMaterials();
		}

		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLSkeletalIndividual::CalcDefaultClassValue() const
{
	if (IsInit())
	{
		if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(ParentActor))
		{
			if (USkeletalMeshComponent* SkMC = SkMA->GetSkeletalMeshComponent())
			{
				FString ClassName = SkMC->SkeletalMesh->GetFullName();
				int32 FindCharPos;
				ClassName.FindLastChar('.', FindCharPos);
				ClassName.RemoveAt(0, FindCharPos + 1);
				ClassName.RemoveFromStart(TEXT("SK_"));
				return ClassName;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SkMC.."),
					*FString(__func__), __LINE__, *SkMA->GetName());
			}
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
	if (HasValidSkeletalMesh() || SetSkeletalMesh())
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
	ClearChildrenIndividualsValue();
	SetIsLoaded(false);
}

// Check if the static mesh component is set
bool USLSkeletalIndividual::HasValidSkeletalMesh() const
{
	return SkeletalMeshComponent && SkeletalMeshComponent->IsValidLowLevel() && !SkeletalMeshComponent->IsPendingKill();
}

// Set sekeletal mesh
bool USLSkeletalIndividual::SetSkeletalMesh()
{
	if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(ParentActor))
	{
		if (USkeletalMeshComponent* SMC = SkMA->GetSkeletalMeshComponent())
		{
			SkeletalMeshComponent = SMC;
			OriginalMaterials = SMC->GetMaterials();
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SkeletalMeshComponent, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s ParentActor is not a SkeletalMeshActor, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
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
	if (!HasValidSkeletalMesh())
	{
		return false;
	}

	// The total number of bone individuals should coincide with the total skeletal bones
	if (BoneIndividuals.Num() + VirtualBoneIndividuals.Num() != SkeletalMeshComponent->GetNumBones())
	{
		return false;
	}

	// Make sure all individuals are valid
	bool bAllBonesAreValid = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsValidLowLevel() || BI->IsPendingKill() || !BI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s invalid bone found.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bAllBonesAreValid = false;
		}
	}
	for (const auto VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsValidLowLevel() || VBI->IsPendingKill() || !VBI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s invalid virtual bone found.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bAllBonesAreValid = false;
		}
	}
	return bAllBonesAreValid;
}

// Create new bone objects
bool USLSkeletalIndividual::CreateChildrenIndividuals()
{
	// Destroy any previous individuals
	DestroyChildrenIndividuals();

	if (!HasValidSkeletalMesh())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's skeletal mesh is not set, cannot create bone individuals.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	for (const auto& BoneData : SkeletalDataAsset->BoneIndexClass)
	{
		if (!BoneData.Value.IsEmpty())
		{
			USLBoneIndividual* BI = NewObject<USLBoneIndividual>(this);
			int32 MatIdx = SkeletalMeshComponent->GetMaterialIndex(FName(*BoneData.Value));
			if (MatIdx == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's a bone will be invalid, no material slot found with the name: %s .."),
					*FString(__FUNCTION__), __LINE__, *GetFullName(), *BoneData.Value);
			}
			BI->PreInit(BoneData.Key, MatIdx);
			BoneIndividuals.Add(BI);
		}
		else
		{
			USLVirtualBoneIndividual* VBI = NewObject<USLVirtualBoneIndividual>(this);
			VBI->PreInit(BoneData.Key, false);
			VirtualBoneIndividuals.Add(VBI);
		}

		// TODO
		//SkeletalMeshComponent->Constraints
	}


	// TODO skeletal joints
	UPhysicsAsset* PA = SkeletalMeshComponent->GetPhysicsAsset();
	for (const auto& CS : PA->ConstraintSetup)
	{

	}

	return HasValidChildrenIndividuals();
}

// Call init on all bones, true if all succesfully init
bool USLSkeletalIndividual::InitChildrenIndividuals()
{
	bool bAllBonesAreInit = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsInit() && !BI->Init(false))
		{
			bAllBonesAreInit = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d bone %s could not be init.."),
				*FString(__FUNCTION__), __LINE__, *BI->GetFullName());
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsInit() && !VBI->Init(false))
		{
			bAllBonesAreInit = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d virtual bone %s could not be init.."),
				*FString(__FUNCTION__), __LINE__, *VBI->GetFullName());
		}
	}
	return bAllBonesAreInit;
}

// Call load on all bones, true if all succesfully loaded
bool USLSkeletalIndividual::LoadChildrenIndividuals()
{
	bool bAllBonesAreLoaded = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsLoaded() && !BI->Load(false, true))
		{
			bAllBonesAreLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d bone %s could not be loaded.."),
				*FString(__FUNCTION__), __LINE__, *BI->GetFullName());
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsLoaded() && !VBI->Load(false, true))
		{
			bAllBonesAreLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d virtual bone %s could not be loaded.."),
				*FString(__FUNCTION__), __LINE__, *VBI->GetFullName());
		}
	}
	return bAllBonesAreLoaded;
}

// Destroy bone individuals
void USLSkeletalIndividual::DestroyChildrenIndividuals()
{
	for (const auto& BI : BoneIndividuals)
	{
		BI->ConditionalBeginDestroy();
	}
	BoneIndividuals.Empty();
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		VBI->ConditionalBeginDestroy();
	}
	VirtualBoneIndividuals.Empty();
}

// Reset bone individuals
void USLSkeletalIndividual::ClearChildrenIndividualsValue()
{
	for (const auto& BI : BoneIndividuals)
	{
		BI->Load(true, false);
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		VBI->Load(true, false);
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
//void USLSkeletalIndividual::BindChildIndividualDelegates(USLBaseIndividual* ChildIndividual)
//{
//	// Bind init change delegate
//	if (!ChildIndividual->OnInitChanged.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildInitChange))
//	{
//		ChildIndividual->OnInitChanged.AddDynamic(this, &USLSkeletalIndividual::OnChildInitChange);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on init delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildIndividual->GetFullName());
//	}
//
//	// Bind load change delegate
//	if (!ChildIndividual->OnLoadedChanged.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildLoadedChange))
//	{
//		ChildIndividual->OnLoadedChanged.AddDynamic(this, &USLSkeletalIndividual::OnChildLoadedChange);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on loaded delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildIndividual->GetFullName());
//	}
//
//	// Bind delegeates cleared 
//	if (!ChildIndividual->OnDelegatesCleared.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildDelegatesCleared))
//	{
//		ChildIndividual->OnDelegatesCleared.AddDynamic(this, &USLSkeletalIndividual::OnChildDelegatesCleared);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on delegates cleared delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildIndividual->GetFullName());
//	}
//
//	/* Values delegates */
//	if (!ChildIndividual->OnNewValue.IsAlreadyBound(this, &USLSkeletalIndividual::OnChildNewValue))
//	{
//		ChildIndividual->OnNewValue.AddDynamic(this, &USLSkeletalIndividual::OnChildNewValue);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s on delegates cleared delegate is already bound, this should not happen.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *ChildIndividual->GetFullName());
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
