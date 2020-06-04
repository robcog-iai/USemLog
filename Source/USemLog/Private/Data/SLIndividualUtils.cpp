// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLindividualUtils.h"
#include "Data/SLBaseIndividual.h"
#include "Data/SLPerceivableIndividual.h"
#include "Data/SLSkeletalIndividual.h"

#include "Data/SLIndividualComponent.h"
#include "Data/SLIndividualVisualInfoComponent.h"

#include "EngineUtils.h"
#include "Kismet2/ComponentEditorUtils.h" // GenerateValidVariableName

#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Vision/SLVisionCamera.h"


// Utils
#include "Utils/SLUUid.h"


// Add new individual component to actor (return true if component has been created or modified)
bool FSLIndividualUtils::AddNewIndividualComponent(AActor* Actor, bool bOverwrite)
{
	if (SupportsIndividualComponents(Actor))
	{
		// Check if individual already exists (avoid duplicates)
		if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s already has an SLIndividualComponent.."),
			//	*FString(__FUNCTION__), __LINE__, *Actor->GetName());
			if (bOverwrite)
			{
				USLIndividualComponent* ExistingComp = CastChecked<USLIndividualComponent>(AC);
				ExistingComp->Init(true);
				ExistingComp->Load(true);
				return true;
			}
			return false;
		}
		else
		{
			// Create new component
			Actor->Modify();

			// Create an appropriate name for the new component (avoid duplicates)
			FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(USLIndividualComponent::StaticClass(), Actor);

			// Get the set of owned components that exists prior to instancing the new component.
			TInlineComponentArray<UActorComponent*> PreInstanceComponents;
			Actor->GetComponents(PreInstanceComponents);

			// Create a new component
			USLIndividualComponent* NewComp = NewObject<USLIndividualComponent>(Actor, NewComponentName, RF_Transactional);

			// Make visible in the components list in the editor
			Actor->AddInstanceComponent(NewComp);
			//Actor->AddOwnedComponent(NewComp);

			NewComp->OnComponentCreated();
			NewComp->RegisterComponent();

			// Register any new components that may have been created during construction of the instanced component, but were not explicitly registered.
			TInlineComponentArray<UActorComponent*> PostInstanceComponents;
			Actor->GetComponents(PostInstanceComponents);
			for (UActorComponent* ActorComponent : PostInstanceComponents)
			{
				if (!ActorComponent->IsRegistered() && ActorComponent->bAutoRegister && !ActorComponent->IsPendingKill() && !PreInstanceComponents.Contains(ActorComponent))
				{
					ActorComponent->RegisterComponent();
				}
			}

			Actor->RerunConstructionScripts();

			NewComp->Init();
			NewComp->Load();

			return true;
		}

	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot add SLIndividualComponent to %s, %s is not a supported actor type.."),
		*FString(__FUNCTION__), __LINE__, *Actor->GetName(), *Actor->GetArchetype()->GetName());
	return false;
}

// Add new visual info component to actor (return true if component has been created or modified)
bool FSLIndividualUtils::AddNewVisualInfoComponent(AActor* Actor, bool bOverwrite)
{
	// Check if it has an individual component
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		// Check if a visual info component already exists
		if (UActorComponent* ACVI = Actor->GetComponentByClass(USLIndividualVisualInfoComponent::StaticClass()))
		{
			if (bOverwrite)
			{
				USLIndividualVisualInfoComponent* ExistingComp = CastChecked<USLIndividualVisualInfoComponent>(ACVI);
				ExistingComp->Init(true);
				ExistingComp->RefreshComponents();
				return true;
			}
			return false;
		}
		else
		{
			// Create new component
			Actor->Modify();

			// Create an appropriate name for the new component (avoid duplicates)
			FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(USLIndividualVisualInfoComponent::StaticClass(), Actor);

			// Get the set of owned components that exists prior to instancing the new component.
			TInlineComponentArray<UActorComponent*> PreInstanceComponents;
			Actor->GetComponents(PreInstanceComponents);

			// Create a new component
			USLIndividualVisualInfoComponent* NewComp = NewObject<USLIndividualVisualInfoComponent>(Actor, NewComponentName, RF_Transactional);

			// Make visible in the components list in the editor
			Actor->AddInstanceComponent(NewComp);
			Actor->AddOwnedComponent(NewComp);

			NewComp->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

			NewComp->OnComponentCreated();
			NewComp->RegisterComponent();

			// Register any new components that may have been created during construction of the instanced component, but were not explicitly registered.
			TInlineComponentArray<UActorComponent*> PostInstanceComponents;
			Actor->GetComponents(PostInstanceComponents);
			for (UActorComponent* ActorComponent : PostInstanceComponents)
			{
				if (!ActorComponent->IsRegistered() && ActorComponent->bAutoRegister && !ActorComponent->IsPendingKill() && !PreInstanceComponents.Contains(ActorComponent))
				{
					ActorComponent->RegisterComponent();
				}
			}

			Actor->RerunConstructionScripts();

			NewComp->Init();
			NewComp->RefreshComponents();

			return true;
		}
	}
	return false;
}

// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
FString FSLIndividualUtils::GetIndividualClassName(AActor* Actor, bool bDefaultToLabelName)
{
	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Actor))
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
					*FString(__func__), __LINE__, *Actor->GetName());
			}
			return ClassName;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SMC.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			return FString();
		}
	}
	else if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(Actor))
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
				*FString(__func__), __LINE__, *Actor->GetName());
			return FString();
		}
	}
	else if (ASLVisionCamera* VCA = Cast<ASLVisionCamera>(Actor))
	{
		static const FString TagType = "SemLog";
		static const FString TagKey = "Class";
		FString ClassName = "View";

		// Check attachment actor
		if (AActor* AttAct = Actor->GetAttachParentActor())
		{
			if (Actor->GetAttachParentSocketName() != NAME_None)
			{
				return Actor->GetAttachParentSocketName().ToString() + ClassName;
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
				*FString(__func__), __LINE__, *Actor->GetName());
			return ClassName;
		}
	}
	else if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(Actor))
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
			else if (PCC->ConstraintInstance.GetAngularSwing1Motion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetAngularSwing2Motion() != ELinearConstraintMotion::LCM_Locked ||
				PCC->ConstraintInstance.GetAngularTwistMotion() != ELinearConstraintMotion::LCM_Locked)
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
	else if (bDefaultToLabelName)
	{
		return Actor->GetActorLabel();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not get the semantic class name for %s .."),
			*FString(__func__), __LINE__, *Actor->GetName());
		return FString();
	}
}

// Create default individual object depending on the owner type (returns nullptr if failed)
UClass* FSLIndividualUtils::CreateIndividualObject(UObject* Outer, AActor* Owner, USLBaseIndividual*& IndividualObject)
{
	UClass* IndividualClass = nullptr;

	// Set semantic individual type depending on owner
	if (Owner->IsA(AStaticMeshActor::StaticClass()))
	{
		IndividualClass = USLPerceivableIndividual::StaticClass();
		IndividualObject = NewObject<USLBaseIndividual>(Outer, IndividualClass);
	}
	else if (Owner->IsA(ASkeletalMeshActor::StaticClass()))
	{
		IndividualClass = USLSkeletalIndividual::StaticClass();
		IndividualObject = NewObject<USLBaseIndividual>(Outer, IndividualClass);
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d unsuported actor type for creating a semantic individual %s-%s.."),
		//	*FString(__FUNCTION__), __LINE__, *Owner->GetClass()->GetName(), *Owner->GetName());
	}
	return IndividualClass;
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


/* Id */
// Write unique id to the actor
bool FSLIndividualUtils::WriteId(AActor* Actor, bool bOverwrite)
{
	if (USLBaseIndividual* SI = GetCastedIndividualObject<USLBaseIndividual>(Actor))
	{
		if (!SI->HasId() || bOverwrite)
		{
			const FString NewId = FSLUuid::NewGuidInBase64Url();
			if (!SI->GetId().Equals(NewId))
			{
				SI->SetId(NewId);
				return true;
			}
		}
	}
	return false;
}

// Clear unique id of the actor
bool FSLIndividualUtils::ClearId(AActor* Actor)
{
	if (USLBaseIndividual* SI = GetCastedIndividualObject<USLBaseIndividual>(Actor))
	{
		if (SI->HasId())
		{
			SI->SetId("");
			return true;
		}
	}
	return false;
}


/* Class */
// Write class name to the actor
bool FSLIndividualUtils::WriteClass(AActor* Actor, bool bOverwrite)
{
	if (USLBaseIndividual* SI = GetCastedIndividualObject<USLBaseIndividual>(Actor))
	{
		if (!SI->HasClass() || bOverwrite)
		{
			const FString NewName = FSLIndividualUtils::GetIndividualClassName(Actor);
			if (!SI->GetClass().Equals(NewName))
			{
				SI->SetClass(NewName);
				return true;
			}
		}
	}
	return false;
}

// Clear class name of the actor
bool FSLIndividualUtils::ClearClass(AActor* Actor)
{
	if (USLBaseIndividual* SI = GetCastedIndividualObject<USLBaseIndividual>(Actor))
	{
		if (SI->HasClass())
		{
			SI->SetClass("");
			return true;
		}
	}
	return false;
}

/* Visual mask */
// Write unique visual masks for all visual individuals in the world
bool FSLIndividualUtils::WriteVisualMasks(UWorld* World, bool bOverwrite)
{
	bool bAddedAny = false;
	TArray<FColor> ConsumedColors = GetConsumedVisualMaskColors(World);
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (USLPerceivableIndividual* VI = GetCastedIndividualObject<USLPerceivableIndividual>(*ActItr))
		{
			bAddedAny = bAddedAny || AddVisualMask(VI, ConsumedColors, bOverwrite);
		}
	}
	return bAddedAny;
}

// Write unique visual masks for visual individuals from the actos in the array
bool FSLIndividualUtils::WriteVisualMasks(const TArray<AActor*>& Actors, UWorld* World, bool bOverwrite)
{
	bool bAddedAny = false;
	TArray<FColor> ConsumedColors = GetConsumedVisualMaskColors(World);
	for (const auto& Act : Actors)
	{
		if (USLPerceivableIndividual* SI = GetCastedIndividualObject<USLPerceivableIndividual>(Act))
		{
			bAddedAny = bAddedAny || AddVisualMask(SI, ConsumedColors, bOverwrite);
		}
	}
	return bAddedAny;
}

// Clear visual mask of the actor
bool FSLIndividualUtils::ClearVisualMask(AActor* Actor)
{
	if (USLPerceivableIndividual* SI = GetCastedIndividualObject<USLPerceivableIndividual>(Actor))
	{
		SI->SetVisualMask("");
		return true;
	}
	return false;
}

/* Private helpers */
// Check if actor supports individual components
bool FSLIndividualUtils::SupportsIndividualComponents(AActor* Actor)
{
	if (Actor->IsA(AStaticMeshActor::StaticClass()))
	{
		return true;
	}
	else if (Actor->IsA(ASkeletalMeshActor::StaticClass()))
	{
		return true;
	}
	return false;
}

// Add visual mask
bool FSLIndividualUtils::AddVisualMask(USLPerceivableIndividual* Individual, TArray<FColor>& ConsumedColors, bool bOverwrite)
{
	static const int32 NumTrials = 100;
	static const int32 MinManhattanDist = 29;

	if (!Individual->HasVisualMask())
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
			Individual->SetVisualMask(NewColor.ToHex());
			return true;
		}
	}
	else if(bOverwrite)
	{
		// Remove previous color from the consumed array
		int32 ConsumedColorIdx = ConsumedColors.Find(FColor::FromHex(Individual->GetVisualMask()));
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
			Individual->SetVisualMask(NewColor.ToHex());
			return true;
		}
	}
	return false;
}

/* Private */
/* Visual mask generation */
// Get all used up visual masks in the world
TArray<FColor> FSLIndividualUtils::GetConsumedVisualMaskColors(UWorld* World)
{
	TArray<FColor> ConsumedColors;

	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		if (USLPerceivableIndividual* VI = GetCastedIndividualObject<USLPerceivableIndividual>(*ActItr))
		{
			if (VI->HasVisualMask())
			{
				ConsumedColors.Add(FColor::FromHex(VI->GetVisualMask()));
			}
		}
	}

	/* Skeletal mesh actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
	}

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
