// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualUtils.h"
#include "Individuals/SLBaseIndividual.h"
#include "Individuals/SLConstraintIndividual.h"
#include "Individuals/SLRigidIndividual.h"
#include "Individuals/SLSkyIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Individuals/SLVirtualViewIndividual.h"

#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualTextInfoComponent.h"

#include "EngineUtils.h"
#include "Kismet2/ComponentEditorUtils.h" // GenerateValidVariableName

#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Vision/SLVirtualCameraView.h"

#include "Atmosphere/AtmosphericFog.h"

// Skeletal data asset search
#include "AssetRegistryModule.h"
#include "Skeletal/SLSkeletalDataAsset.h"

// Utils
#include "Utils/SLUuid.h"

#include <bson/bson.h>
#if SL_WITH_LIBMONGO_C
THIRD_PARTY_INCLUDES_START
	#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include <bson/bson.h>
	#include "Windows/HideWindowsPlatformTypes.h"
	#else
	#include <bson/bson.h>
	#endif // #if PLATFORM_WINDOWS
THIRD_PARTY_INCLUDES_END
#endif // SL_WITH_LIBMONGO_C

// Get the individual component from the actor (nullptr if it does not exist)
USLIndividualComponent* FSLIndividualUtils::GetIndividualComponent(AActor* Owner)
{
	if (UActorComponent* AC =Owner->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		return CastChecked<USLIndividualComponent>(AC);
	}
	return nullptr;
}

// Get the individual object from the actor (nullptr if it does not exist)
USLBaseIndividual* FSLIndividualUtils::GetIndividualObject(AActor* Owner)
{
	if (USLIndividualComponent* IC = GetIndividualComponent(Owner))
	{
		return IC->GetIndividualObject();
	}
	return nullptr;
}

// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
FString FSLIndividualUtils::GetIndividualClassName(USLIndividualComponent* IndividualComponent, bool bDefaultToLabelName)
{
	AActor* CompOwner = IndividualComponent->GetOwner();
	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(CompOwner))
	{
		if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			FString ClassName = SMC->GetStaticMesh()->GetFullName();
			int32 FindCharPos;
			ClassName.FindLastChar('.', FindCharPos);
			ClassName.RemoveAt(0, FindCharPos + 1);
			if (!ClassName.RemoveFromStart(TEXT("SM_")))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s StaticMesh has no SM_ prefix in its name.."),
					*FString(__func__), __LINE__, *CompOwner->GetName());
			}
			return ClassName;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SMC.."),
				*FString(__func__), __LINE__, *CompOwner->GetName());
			return FString();
		}
	}
	else if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(CompOwner))
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
				*FString(__func__), __LINE__, *CompOwner->GetName());
			return FString();
		}
	}
	else if (ASLVirtualCameraView* VCA = Cast<ASLVirtualCameraView>(CompOwner))
	{
		static const FString TagType = "SemLog";
		static const FString TagKey = "Class";
		FString ClassName = "View";
	
		// Check attachment actor
		if (AActor* AttAct = CompOwner->GetAttachParentActor())
		{
			if (CompOwner->GetAttachParentSocketName() != NAME_None)
			{
				return CompOwner->GetAttachParentSocketName().ToString() + ClassName;
			}
			else
			{
				FString AttParentClass = FSLTagIO::GetValue(AttAct, TagType, TagKey);
				if (!AttParentClass.IsEmpty())
				{
					return AttParentClass + ClassName;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Attached parent %s has no semantic class (yet?).."),
						*FString(__func__), __LINE__, *AttAct->GetName());
					return ClassName;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not attached to any actor.."),
				*FString(__func__), __LINE__, *CompOwner->GetName());
			return ClassName;
		}
	}
	else if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(CompOwner))
	{
		FString ClassName = "Joint";
	
		if (UPhysicsConstraintComponent* PCC = PCA->GetConstraintComp())
		{
			if (PCC->ConstraintInstance.GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
			{
				return "Linear" + ClassName;
			}
			else if (PCC->ConstraintInstance.GetAngularSwing1Motion() != EAngularConstraintMotion::ACM_Locked ||
				PCC->ConstraintInstance.GetAngularSwing2Motion() != EAngularConstraintMotion::ACM_Locked ||
				PCC->ConstraintInstance.GetAngularTwistMotion() != EAngularConstraintMotion::ACM_Locked)
			{
				return "Revolute" + ClassName;
			}
			else
			{
				return "Fixed" + ClassName;
			}
		}
		return ClassName;
	}
	else if (AAtmosphericFog* AAF = Cast<AAtmosphericFog>(CompOwner))
	{
		return "AtmosphericFog";
	}
	else if (CompOwner->GetName().Contains("SkySphere"))
	{
		return "SkySphere";
	}
	else if (bDefaultToLabelName)
	{
		return CompOwner->GetActorLabel();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not get the semantic class name for %s .."),
			*FString(__func__), __LINE__, *CompOwner->GetName());
		return FString();
	}
}

// Check if actor supports individual components
bool FSLIndividualUtils::CanHaveIndividualComponent(AActor* Actor)
{
	return Actor->IsA(AStaticMeshActor::StaticClass())
		|| Actor->IsA(ASkeletalMeshActor::StaticClass())
		|| Actor->IsA(APhysicsConstraintActor::StaticClass())
		|| Actor->IsA(AAtmosphericFog::StaticClass())
		|| Actor->IsA(ASLVirtualCameraView::StaticClass())
		|| Actor->GetName().Contains("SkySphere");
}

// Create default individual object depending on the owner type (returns nullptr if failed)
USLBaseIndividual* FSLIndividualUtils::CreateIndividualObject(UObject* Outer, AActor* Owner)
{
	// Set semantic individual type depending on owner
	if (Owner->IsA(AStaticMeshActor::StaticClass()))
	{
		return NewObject<USLBaseIndividual>(Outer, USLRigidIndividual::StaticClass());
	}
	else if (Owner->IsA(APhysicsConstraintActor::StaticClass()))
	{
		return NewObject<USLBaseIndividual>(Outer, USLConstraintIndividual::StaticClass());
	}
	else if (Owner->IsA(ASLVirtualCameraView::StaticClass()))
	{
		return NewObject<USLBaseIndividual>(Outer, USLVirtualViewIndividual::StaticClass());
	}
	else if (Owner->IsA(ASkeletalMeshActor::StaticClass()))
	{
		return NewObject<USLBaseIndividual>(Outer, USLSkeletalIndividual::StaticClass());
	}
	else if (Owner->IsA(AAtmosphericFog::StaticClass()) || Owner->GetName().Contains("SkySphere"))
	{
		return NewObject<USLBaseIndividual>(Outer, USLSkyIndividual::StaticClass());
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d unsuported actor type for creating a semantic individual %s-%s.."),
		//	*FString(__FUNCTION__), __LINE__, *Owner->GetClass()->GetName(), *Owner->GetName());
	}
	return nullptr;
}

// Convert individual to the given type
bool FSLIndividualUtils::ConvertIndividualObject(USLBaseIndividual*& IndividualObject, TSubclassOf<class USLBaseIndividual> ConvertToClass)
{
	//if (ConvertToClass && IndividualObject && !IndividualObject->IsPendingKill())
	//{
	//	if (IndividualObject->GetClass() != ConvertToClass)
	//	{
	//		// TODO cache common individual data to copy to the newly created individual
	//		UObject* Outer = IndividualObject->GetOuter();
	//		IndividualObject->ConditionalBeginDestroy();
	//		IndividualObject = NewObject<USLBaseIndividual>(Outer, ConvertToClass);
	//		return true;
	//	}
	//	else
	//	{
	//		//UE_LOG(LogTemp, Error, TEXT("%s::%d Same class type (%s-%s), no conversion is required.."),
	//		//	*FString(__FUNCTION__), __LINE__, *IndividualObject->GetClass()->GetName(), *ConvertToClass->GetName());
	//	}
	//}
	return false;
}

// Generate a new bson oid as string, empty string if fails
FString FSLIndividualUtils::NewOIdAsString()
{
#if SL_WITH_LIBMONGO_C
	bson_oid_t new_oid;
	bson_oid_init(&new_oid, NULL);
	char oid_str[25];
	bson_oid_to_string(&new_oid, oid_str);
	return FString(UTF8_TO_TCHAR(oid_str));
#elif
	return FString();
#endif // #if PLATFORM_WINDOWS
	return FString();
}

// Find the skeletal data asset for the individual
USLSkeletalDataAsset* FSLIndividualUtils::FindSkeletalDataAsset(AActor* Owner)
{
	if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(Owner))
	{
		if (USkeletalMeshComponent* SkMC = SkMA->GetSkeletalMeshComponent())
		{
			// Get skeletal mesh name (SK_ABC)
			FString SkelAssetName = SkMC->SkeletalMesh->GetFullName();
			int32 FindCharPos;
			SkelAssetName.FindLastChar('.', FindCharPos);
			SkelAssetName.RemoveAt(0, FindCharPos + 1);

			// Find data asset (SLSK_ABC)
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			TArray<FAssetData> AssetData;
			FARFilter Filter;
			Filter.PackagePaths.Add("/USemLog/Skeletal");
			Filter.ClassNames.Add(USLSkeletalDataAsset::StaticClass()->GetFName());
			AssetRegistryModule.Get().GetAssets(Filter, AssetData);

			// Search for the results
			for (const auto& AD : AssetData)
			{
				if (AD.AssetName.ToString().Contains(SkelAssetName))
				{
					return Cast<USLSkeletalDataAsset>(AD.GetAsset());
				}
			}
		}
	}
	return nullptr;
}

//int32 FSLIndividualUtils::CreateIndividualComponents(UWorld* World)
//{
//	return int32();
//}
//
//int32 FSLIndividualUtils::CreateIndividualComponents(const TArray<AActor*>& Actors)
//{
//	return int32();
//}

/* Id */
// Write unique id to the actor
bool FSLIndividualUtils::WriteId(USLIndividualComponent* IndividualComponent, bool bOverwrite)
{
	bool RetVal = false;
	if (USLBaseIndividual* SI = IndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
	{
		if (!SI->IsIdValueSet() || bOverwrite)
		{
			const FString NewId = FSLUuid::NewGuidInBase64Url();
			if (!SI->GetIdValue().Equals(NewId))
			{
				SI->SetIdValue(NewId);
				RetVal = true;
			}
		}

		if (!SI->IsOIdValueSet() || bOverwrite)
		{
			const FString NewOId = NewOIdAsString();
			if (!SI->GetOIdValue().Equals(NewOId))
			{
				SI->SetOIdValue(NewOId);
				RetVal = true;
			}
		}
	}
	return RetVal;
}

// Clear unique id of the actor
bool FSLIndividualUtils::ClearId(USLIndividualComponent* IndividualComponent)
{
	bool RetVal = false;
	if (USLBaseIndividual* SI = IndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
	{
		if (SI->IsIdValueSet())
		{
			SI->ClearIdValue();
			RetVal = true;
		}

		if (SI->IsOIdValueSet())
		{
			SI->ClearOIdValue();
			RetVal = true;
		}
	}
	return false;
}


/* Class */
// Write class name to the actor
bool FSLIndividualUtils::WriteClass(USLIndividualComponent* IndividualComponent, bool bOverwrite)
{
	if (USLBaseIndividual* SI = IndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
	{
		if (!SI->IsClassValueSet() || bOverwrite)
		{
			const FString NewName = FSLIndividualUtils::GetIndividualClassName(IndividualComponent);
			if (!SI->GetClassValue().Equals(NewName))
			{
				SI->SetClassValue(NewName);
				return true;
			}
		}
	}
	return false;
}

// Clear class name of the actor
bool FSLIndividualUtils::ClearClass(USLIndividualComponent* IndividualComponent)
{
	if (USLBaseIndividual* SI = IndividualComponent->GetCastedIndividualObject<USLBaseIndividual>())
	{
		if (SI->IsClassValueSet())
		{
			SI->ClearClassValue();
			return true;
		}
	}
	return false;
}


/* Visual mask */
// Write unique visual masks for all visual individuals in the world
int32 FSLIndividualUtils::WriteVisualMasks(const TSet<USLIndividualComponent*>& IndividualComponents, bool bOverwrite)
{
	int32 NumNewMasks = 0;
	TArray<FColor> ConsumedColors = GetConsumedVisualMaskColors(IndividualComponents);
	for (const auto& IC : IndividualComponents)
	{
		if (USLPerceivableIndividual* VI = IC->GetCastedIndividualObject<USLPerceivableIndividual>())
		{
			if (AddVisualMask(VI, ConsumedColors, bOverwrite))
			{
				NumNewMasks++;
			}
		}

		// TODO check for children
	}
	return NumNewMasks;
}

// Write unique visual masks for visual individuals from the actos in the array
int32 FSLIndividualUtils::WriteVisualMasks(const TSet<USLIndividualComponent*>& IndividualComponentsSelection,
	const TSet<USLIndividualComponent*>& RegisteredIndividualComponents,
	bool bOverwrite)
{
	int32 NumNewMasks = 0;
	TArray<FColor> ConsumedColors = GetConsumedVisualMaskColors(RegisteredIndividualComponents);
	for (const auto& IC : IndividualComponentsSelection)
	{
		if (USLPerceivableIndividual* VI = IC->GetCastedIndividualObject<USLPerceivableIndividual>())
		{
			if (AddVisualMask(VI, ConsumedColors, bOverwrite))
			{
				NumNewMasks++;
			}
		}
		
		// TODO check for children
		if (USLSkeletalIndividual* SkI = IC->GetCastedIndividualObject<USLSkeletalIndividual>())
		{

		}
	}
	return NumNewMasks;
}

bool FSLIndividualUtils::ClearVisualMask(USLIndividualComponent* IndividualComponent)
{
	if (USLPerceivableIndividual* SI = IndividualComponent->GetCastedIndividualObject<USLPerceivableIndividual>())
	{
		SI->ClearVisualMaskValue();
		return true;
	}

	// TODO skeletal
	// TODO robot

	return false;
}

/* Private helpers */
// Add visual mask
bool FSLIndividualUtils::AddVisualMask(USLPerceivableIndividual* Individual, TArray<FColor>& ConsumedColors, bool bOverwrite)
{
	static const int32 NumTrials = 100;
	static const int32 MinManhattanDist = 29;

	if (!Individual->IsVisualMaskValueSet())
	{
		// Generate new color
		FColor NewColor = CreateNewUniqueColorRand(ConsumedColors, NumTrials, MinManhattanDist);
		if (NewColor == FColor::Black)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a new visual mask for %s .."),
				*FString(__func__), __LINE__, *Individual->GetOuter()->GetName());
			return false;
		}
		else
		{
			Individual->SetVisualMaskValue(NewColor.ToHex());
			return true;
		}
	}
	else if(bOverwrite)
	{
		// Remove previous color from the consumed array
		int32 ConsumedColorIdx = ConsumedColors.Find(FColor::FromHex(Individual->GetVisualMaskValue()));
		if (ConsumedColorIdx != INDEX_NONE)
		{
			ConsumedColors.RemoveAt(ConsumedColorIdx);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d To be overwritten color of %s is not in the consumed colors array, this should not happen  .."),
				*FString(__func__), __LINE__, *Individual->GetOuter()->GetName());
		}

		// Generate new color
		FColor NewColor = CreateNewUniqueColorRand(ConsumedColors, NumTrials, MinManhattanDist);
		if (NewColor == FColor::Black)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a new visual mask for %s .."),
				*FString(__func__), __LINE__, *Individual->GetOuter()->GetName());
			return false;
		}
		else
		{
			Individual->SetVisualMaskValue(NewColor.ToHex());
			return true;
		}
	}
	return false;
}

/* Private */
/* Visual mask generation */
// Get all used up visual masks in the world
TArray<FColor> FSLIndividualUtils::GetConsumedVisualMaskColors(const TSet<USLIndividualComponent*>& IndividualComponents)
{
	TArray<FColor> ConsumedColors;

	/* Static mesh actors */
	for (const auto& IC : IndividualComponents)
	{
		if (USLPerceivableIndividual* VI = IC->GetCastedIndividualObject<USLPerceivableIndividual>())
		{
			if (VI->IsVisualMaskValueSet())
			{
				ConsumedColors.Add(FColor::FromHex(VI->GetVisualMaskValue()));
			}
		}
	}

	/* Skeletal mesh actors */
	// TODO

	/* Robot actors */
	// TODO

	return ConsumedColors;
}

// Create a new unique color by randomization
FColor FSLIndividualUtils::CreateNewUniqueColorRand(TArray<FColor>& ConsumedColors, int32 NumTrials, int32 MinManhattanDist)
{
	// Constants
	static const uint8 MinDistToBlack = 37;
	static const uint8 MinDistToWhite = 23;

	for (int32 TrialIdx = 0; TrialIdx < NumTrials; ++TrialIdx)
	{
		// Generate a random color that differs of black
		//FColor RC = FColor::MakeRandomColor(); // Pretty colors, but not many
		FColor RandColor = CreateRandomRGBColor();

		// Avoid very dark or very bright colors
		if (AreColorsEqual(RandColor, FColor::Black, MinDistToBlack) || 
			AreColorsEqual(RandColor, FColor::White, MinDistToWhite))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Got a very dark or very bright (reserved) color, hex=%s, trying again.."),
				*FString(__func__), __LINE__, *RandColor.ToHex());
			continue;
		}

		/* Nested lambda for the array FindByPredicate functionality */
		const auto EqualWithToleranceLambda = [RandColor, MinManhattanDist](const FColor& Item)
		{
			return AreColorsEqual(RandColor, Item, MinManhattanDist);
		};

		// Check that the randomly generated color is not in the consumed color array
		if (!ConsumedColors.FindByPredicate(EqualWithToleranceLambda))
		{
			ConsumedColors.Emplace(RandColor);
			return RandColor;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a unique color, saving as black.."), *FString(__func__), __LINE__);
	return FColor::Black;
}
