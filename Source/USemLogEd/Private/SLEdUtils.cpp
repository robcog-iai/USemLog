// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)


#include "SLEdUtils.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "AssetRegistryModule.h"

// SL
#include "Editor/SLSemanticMapWriter.h"
#include "Vision/SLVisionCamera.h"
#include "Data/SLIndividualComponent.h"
#include "SLManager.h"

// Utils
#include "Utils/SLTagIO.h"
#include "Utils/SLUUid.h"

// Write the semantic map
void FSLEdUtils::WriteSemanticMap(UWorld* World, bool bOverwrite)
{
	FSLSemanticMapWriter SemMapWriter;
	FString TaskDir;

	for (TActorIterator<ASLManager> ActItr(World); ActItr; ++ActItr)
	{
		TaskDir = *ActItr->GetTaskId();
		break;
	}
	if(TaskDir.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the semantic manager to read the task id, set to default.."),
			*FString(__func__), __LINE__);
		TaskDir = "DefaultTaskId";
	}
	
	// Generate map and write to file
	SemMapWriter.WriteToFile(World, ESLOwlSemanticMapTemplate::IAIKitchen, TaskDir, TEXT("SemanticMap"), bOverwrite);
}


// Write unique IDs
void FSLEdUtils::WriteUniqueIds(UWorld* World, bool bOverwrite)
{
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		AddUniqueId(*ActItr, bOverwrite);
	}
}

void FSLEdUtils::WriteUniqueIds(const TArray<AActor*>& Actors, bool bOverwrite)
{
	for (const auto& Act : Actors)
	{
		AddUniqueId(Act, bOverwrite);
	}
}


// Write class names
void FSLEdUtils::WriteClassNames(UWorld* World, bool bOverwrite)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("Class");

	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		FString ClassName = GetClassName(*ActItr);
		if (!ClassName.IsEmpty())
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, ClassName, bOverwrite);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not get the class name for %s.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
	}
}

void FSLEdUtils::WriteClassNames(const TArray<AActor*>& Actors, bool bOverwrite)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("Class");

	for (const auto& Act : Actors)
	{
		FString ClassName = GetClassName(Act);
		if (!ClassName.IsEmpty())
		{
			FSLTagIO::AddKVPair(Act, TagType, TagKey, ClassName, bOverwrite);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not get the class name for %s.."),
				*FString(__func__), __LINE__, *Act->GetName());
		}
	}
}


// Write unique visual masks
void FSLEdUtils::WriteVisualMasks(UWorld* World, bool bOverwrite)
{
	WriteRandomlyGeneratedVisualMasks(World, bOverwrite);
	// TODO incremental
}

void FSLEdUtils::WriteVisualMasks(const TArray<AActor*>& Actors, UWorld* World, bool bOverwrite)
{
	WriteRandomlyGeneratedVisualMasks(Actors, World, bOverwrite);
	// TODO incremental
}


// Remove all tag keys
void FSLEdUtils::RemoveTagKey(UWorld* World, const FString& TagType, const FString& TagKey)
{
	FSLTagIO::RemoveWorldKVPairs(World, TagType, TagKey);
}

void FSLEdUtils::RemoveTagKey(const TArray<AActor*>& Actors, const FString& TagType, const FString& TagKey)
{
	for (const auto& Act : Actors)
	{
		FSLTagIO::RemoveKVPair(Act, TagType, TagKey);
	}
}


// Remove all tags of the "SemLog" type
void FSLEdUtils::RemoveTagType(UWorld* World, const FString& TagType)
{
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		int32 Pos = INDEX_NONE;
		if (FSLTagIO::HasType(*ActItr, TagType, &Pos))
		{
			ActItr->Modify();
			ActItr->Tags.RemoveAt(Pos);
		}
	}
}

void FSLEdUtils::RemoveTagType(const TArray<AActor*>& Actors, const FString& TagType)
{
	for (const auto& Act : Actors)
	{
		int32 Pos = INDEX_NONE;
		if (FSLTagIO::HasType(Act, TagType, &Pos))
		{
			Act->Modify();
			Act->Tags.RemoveAt(Pos);
		}
	}
}


// Add semantic monitor components to the actors
void FSLEdUtils::AddSemanticMonitorComponents(UWorld* World, bool bOverwrite)
{
	//// Iterate only static mesh actors
	//for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	//{
	//	// Continue only if a valid mesh component is available
	//	if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
	//	{
	//		// Ignore if actor is not tagged
	//		if (FTags::HasKey(*ActItr, "SemLog", "Class"))
	//		{
	//			// Continue if no previous components are created
	//			TArray<USLContactBox*> Comps;
	//			ActItr->GetComponents<USLContactBox>(Comps);
	//			//if (Comps.Num() == 0)
	//			//{
	//			//	USLContactBox* Comp = NewObject<USLContactBox>(*ActItr);
	//			//	Comp->RegisterComponent();
	//			//	/*FTransform T;
	//			//	ActItr->AddComponent("USLContactBox", false, T, USLContactBox::StaticClass());*/
	//			//}
	//		}
	//	}
	//}
}

void FSLEdUtils::AddSemanticMonitorComponents(const TArray<AActor*>& Actors, bool bOverwrite)
{
}


// Add semantic data components to the actors
void FSLEdUtils::AddSemanticDataComponents(UWorld* World, bool bOverwrite)
{
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		//if (ActItr->GetComponentByClass(USLIndividualComponent::StaticClass()))
		//{
		//	continue;
		//}
		//USLIndividualComponent* NewComp = NewObject<USLIndividualComponent>(*ActItr);
		//NewComp->RegisterComponent();
		//if (NewComp->IsRegistered())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is successfully registered"), *FString(__FUNCTION__), __LINE__, *NewComp->GetName());
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d %s FAILED to register"), *FString(__FUNCTION__), __LINE__, *NewComp->GetName());
		//}
		//ActItr->AddOwnedComponent(NewComp);
		//ActItr->AddInstanceComponent(NewComp);
		//ActItr->Modify();
	}

	//for (ULevelStreaming* LevelStreaming : World->GetStreamingLevels())
	//{
	//	if (LevelStreaming && LevelStreaming->IsLevelVisible())
	//	{
	//		if (ULevel* Level = LevelStreaming->GetLoadedLevel())
	//		{
	//			// Iterate method 1
	//			for (TActorIterator<AStaticMeshActor> ActorItr(Level->GetWorld()); ActorItr; ++ActorItr)
	//			{
	//				// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
	//				AStaticMeshActor* Mesh = *ActorItr;
	//				Mesh->AddActorLocalOffset(FVector(500, 500, 500));
	//			}
	//			// Iterate method 2
	//			for (AActor* Actor : Level->Actors)
	//			{
	//				// Store quick map of id to actor pointer
	//				if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Actor))
	//				{
	//					AsSMA->AddActorLocalOffset(FVector(1000, 1000, 100));
	//				}
	//			}
	//		}
	//	}
	//}
}

void FSLEdUtils::AddSemanticDataComponents(const TArray<AActor*>& Actors, bool bOverwrite)
{
}


// Enable overlaps on actors
void FSLEdUtils::EnableOverlaps(UWorld* World)
{
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			SMC->SetGenerateOverlapEvents(true);
		}
	}
}

void FSLEdUtils::EnableOverlaps(const TArray<AActor*>& Actors)
{
	for (const auto& Act : Actors)
	{
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Act))
		{
			if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
			{
				SMC->SetGenerateOverlapEvents(true);
			}
		}
	}
}


// Toggle between showing the semantic data of the entities in the world
void FSLEdUtils::ShowSemanticData(UWorld* World)
{
	//for (TActorIterator<AActor> It(World); It; ++It)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t\t Act=%s;"),
	//		*FString(__FUNCTION__), __LINE__, *It->GetName());
	//}

	//UE_LOG(LogTemp, Error, TEXT("%s::%d **********************"),
	//	*FString(__FUNCTION__), __LINE__);

	//for (ULevelStreaming* LevelStreaming : World->GetStreamingLevels())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Streaming level=%s;"),
	//		*FString(__FUNCTION__), __LINE__, *LevelStreaming->GetName());
	//	if (LevelStreaming && LevelStreaming->IsLevelVisible())
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Is visible=%s;"),
	//			*FString(__FUNCTION__), __LINE__, *LevelStreaming->GetName());
	//		if (ULevel* Level = LevelStreaming->GetLoadedLevel())
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t Loaded level=%s;"),
	//				*FString(__FUNCTION__), __LINE__, *Level->GetName());
	//			for (AActor* Actor : Level->Actors)
	//			{
	//				if (Actor)
	//				{
	//					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Act=%s;"),
	//						*FString(__FUNCTION__), __LINE__, *Actor->GetName());
	//				}
	//				else
	//				{
	//					UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t Act=nullptr;"),
	//						*FString(__FUNCTION__), __LINE__);
	//				}



	//				//// Make sure the actor does not have a component already
	//				//if (Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
	//				//{
	//				//	//USLIndividualComponent* SemanticDataComponent = NewObject<USLIndividualComponent>(USLIndividualComponent::StaticClass(), Actor);
	//				//	//UE_LOG(LogTemp, Error, TEXT("%s::%d %s received a new semantic data component (%s).."), *FString(__FUNCTION__), __LINE__, *Actor->GetName(), *SemanticDataComponent->GetName());
	//				//}
	//				//else
	//				//{
	//				//	//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s already has a semantic data component.."), *FString(__FUNCTION__), __LINE__, *Actor->GetName());
	//				//}
	//			}


	//			UE_LOG(LogTemp, Error, TEXT("%s::%d ----"), *FString(__FUNCTION__), __LINE__);

	//			// Iterate method 1
	//			for (TActorIterator<AActor> ActorItr(Level->GetWorld()); ActorItr; ++ActorItr)
	//			{
	//				UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Act=%s;"),
	//					*FString(__FUNCTION__), __LINE__, *ActorItr->GetName());
	//			}
	//		}
	//	}
	//}
}

void FSLEdUtils::ShowSemanticData(const TArray<AActor*>& Actors)
{
}


// Enable all materials for instanced static mesh rendering
void FSLEdUtils::EnableAllMaterialsForInstancedStaticMesh()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AllAsset;
	AssetRegistryModule.Get().GetAssetsByPath(TEXT("/Game/"), AllAsset, true, false);

	for (FAssetData Data : AllAsset)
	{
		if (Data.AssetClass.ToString().Equals(TEXT("Material")))
		{
			UMaterial* Material = Cast<UMaterial>(Data.GetAsset());
			if (!Material->bUsedWithInstancedStaticMeshes)
			{
				Material->bUsedWithInstancedStaticMeshes = true;
				Data.GetPackage()->MarkPackageDirty();
				UE_LOG(LogTemp, Error, TEXT("%s::%d Material: %s is enabled for instanced static mesh.."), *FString(__func__), __LINE__, *Data.GetPackage()->GetFName().ToString());
			}
		}
	}

}


// Add unique id if the actor is a known type
bool FSLEdUtils::AddUniqueId(AActor* Actor, bool bOverwrite)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("Id");

	/* SMA */
	if (Actor->IsA(AStaticMeshActor::StaticClass()))
	{
		return FSLTagIO::AddKVPair(Actor, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
	}

	/* SkMA */
	if (Actor->IsA(ASkeletalMeshActor::StaticClass()))
	{
		FSLTagIO::AddKVPair(Actor, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);

		// Get the semantic data component containing the semantics (class names mask colors) about the bones
		if (UActorComponent* AC = Actor->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
		{
			// Load existing visual mask values from the skeletal data
			USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
			for (auto& Pair : SkDC->SemanticBonesData)
			{
				// Double check if bone has a semantic class
				if (!Pair.Value.IsClassSet())
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Semantic bones should have a class name set.."), *FString(__func__), __LINE__);
					continue;
				}

				// Check if the bone has id data
				if (bOverwrite)
				{
					Pair.Value.Id = FSLUuid::NewGuidInBase64Url();

					// Add the data to the map used at by the metadatalogger as well
					if (SkDC->AllBonesData.Contains(Pair.Key))
					{
						SkDC->AllBonesData[Pair.Key].Id = Pair.Value.Id;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
							*FString(__func__), __LINE__, *Pair.Key.ToString());
					}
				}
				else if (Pair.Value.Id.IsEmpty())
				{
					Pair.Value.Id = FSLUuid::NewGuidInBase64Url();

					// Add the data to the map used at runtime as well
					if (SkDC->AllBonesData.Contains(Pair.Key))
					{
						SkDC->AllBonesData[Pair.Key].Id = Pair.Value.Id;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
							*FString(__func__), __LINE__, *Pair.Key.ToString());
					}
				}
			}			
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			return false;
		}
	}

	/* Joints */
	if (Actor->IsA(APhysicsConstraintActor::StaticClass()))
	{
		return FSLTagIO::AddKVPair(Actor, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);		
	}

	/* Vision cameras */
	if (Actor->IsA(ASLVisionCamera::StaticClass()))
	{
		return FSLTagIO::AddKVPair(Actor, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
	}

	// Unkown actor type
	return false;
}

// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
FString FSLEdUtils::GetClassName(AActor* Actor, bool bDefaultToLabelName)
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not get the semantic class for %s .."),
			*FString(__func__), __LINE__, *Actor->GetName());
		return FString();
	}
}

// Generate unique visual masks using randomization
void FSLEdUtils::WriteRandomlyGeneratedVisualMasks(UWorld* World, bool bOverwrite)
{
	// TArray is used instead of TSet, because of the FindByPredicate funcionality
	TArray<FColor> ConsumedColors;

	// Load already used colors to avoid generating similar ones
	ConsumedColors = GetConsumedVisualMaskColors(World);

	// Add unique mask avoiding existing ones
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		AddUniqueVisualMask(*ActItr, ConsumedColors, bOverwrite);
	}
}

void FSLEdUtils::WriteRandomlyGeneratedVisualMasks(const TArray<AActor*>& Actors, UWorld* World, bool bOverwrite)
{
	// TArray is used instead of TSet, because of the FindByPredicate funcionality
	TArray<FColor> ConsumedColors;

	// Load already used colors to avoid generating similar ones
	ConsumedColors = GetConsumedVisualMaskColors(World);

	// Add unique mask avoiding existing ones
	for (const auto& Act : Actors)
	{
		AddUniqueVisualMask(Act, ConsumedColors, bOverwrite);
	}
}

// Generate unique visual masks using incremental heuristic
void FSLEdUtils::WriteIncrementallyGeneratedVisualMasks(UWorld* World, bool bOverwrite)
{
	// TODO

		//auto GenerateUniqueColorLambda = [](const uint8 Step, FColor& ColorIdx)->FString
	//{
	//	const uint8 StepFrom255 = 255 - Step;
	//	if (ColorIdx.B > StepFrom255)
	//	{
	//		ColorIdx.B = ColorIdx.B - StepFrom255;
	//		if (ColorIdx.G > StepFrom255)
	//		{
	//			ColorIdx.G = ColorIdx.G - StepFrom255;
	//			if (ColorIdx.R > StepFrom255)
	//			{
	//				ColorIdx = FColor::White;
	//				UE_LOG(LogTemp, Error, TEXT("%s::%d Reached the maximum possible color values.."), *FString(__func__), __LINE__);
	//				return FColor::Black.ToHex();;
	//			}
	//			else
	//			{
	//				ColorIdx.R += Step;
	//				return ColorIdx.ToHex();;
	//			}
	//		}
	//		else
	//		{
	//			ColorIdx.G += Step;
	//			return ColorIdx.ToHex();;
	//		}
	//	}
	//	else
	//	{
	//		ColorIdx.B += Step;
	//		return ColorIdx.ToHex();;
	//	}
	//};

	//// Generated colors params
	//const uint8 Tolerance = 27;
	//FColor CIdx(0, 0, 0, 255);

	//// Lambda to shuffle the unqiue colors array
	//auto ArrayShuffleLambda = [](const TArray<FString>& Colors)
	//{
	//	int32 LastIndex = Colors.Num() - 1;
	//	for (int32 i = 0; i < LastIndex; ++i)
	//	{
	//		int32 Index = FMath::RandRange(0, LastIndex);
	//		if (i != Index)
	//		{
	//			const_cast<TArray<FString>*>(&Colors)->Swap(i, Index);
	//		}
	//	}
	//};

	//// Add all possible colors to an array and shuffle them (avoid having similar color shades next to each other)
	//TArray<FString> UniqueColors;
	//bool bIsColorBlack = false;
	//while (!bIsColorBlack)
	//{
	//	FString UC = GenerateUniqueColorLambda(Tolerance, CIdx);
	//	if (UC.Equals(FColor::Black.ToHex()))
	//	{
	//		bIsColorBlack = true;
	//	}
	//	else
	//	{
	//		UniqueColors.Add(UC);
	//	}
	//}

	//// Shuffle the array
	//ArrayShuffleLambda(UniqueColors);
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d UniqueColorsNum() = %d"), *FString(__func__), __LINE__, UniqueColors.Num());

	//// Iterate only static mesh actors
	//uint32 UsedColors = 0;
	//for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	//{
	//	// Continue only if a valid mesh component is available
	//	if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
	//	{
	//		if (bOverwriteVisualMaskValues)
	//		{
	//			// Check if the actor or the component is semantically annotated
	//			if (FTags::HasKey(*ActItr, "SemLog", "Class"))
	//			{
	//				if (UniqueColors.Num() == 0)
	//				{
	//					UE_LOG(LogTemp, Error, TEXT("%s::%d [Actor] ActItr=%s; No more unique colors, saving as black"),
	//						*FString(__func__), __LINE__, *ActItr->GetName());
	//					FTags::AddKeyValuePair(*ActItr, "SemLog", "VisMask", FColor::Black.ToHex(), true);
	//				}
	//				else
	//				{
	//					const FString ColorStr = UniqueColors.Pop(false);
	//					FTags::AddKeyValuePair(*ActItr, "SemLog", "VisMask", ColorStr, true);
	//					UsedColors++;
	//					UE_LOG(LogTemp, Warning, TEXT("%s::%d [Actor] ActItr=%s; Color=%s; \t\t\t\t [%d/%d] "),
	//						*FString(__func__), __LINE__, *ActItr->GetName(), *ColorStr, UsedColors, UniqueColors.Num());
	//				}
	//			}
	//			else if (FTags::HasKey(SMC, "SemLog", "Class"))
	//			{
	//				if (UniqueColors.Num() == 0)
	//				{
	//					FTags::AddKeyValuePair(SMC, "SemLog", "VisMask", FColor::Black.ToHex(), true);
	//					UE_LOG(LogTemp, Error, TEXT("%s::%d [Comp] Owner=%s; No more unique colors, saving as black"),
	//						*FString(__func__), __LINE__, *ActItr->GetName());
	//				}
	//				else
	//				{
	//					const FString ColorStr = UniqueColors.Pop(false);
	//					FTags::AddKeyValuePair(SMC, "SemLog", "VisMask", ColorStr, true);
	//					UsedColors++;
	//					UE_LOG(LogTemp, Warning, TEXT("%s::%d [Comp] Owner=%s; Color=%s; \t\t\t\t [%d/%d] "),
	//						*FString(__func__), __LINE__, *ActItr->GetName(), *ColorStr, UsedColors, UniqueColors.Num());
	//				}
	//			}
	//		}
	//		else
	//		{
	//			// TODO not implemented
	//			// Load all existing values into the array first
	//			// then start adding new values with AddUnique
	//		}
	//	}
	//}
	//
	//// Iterate skeletal data components
	//for (TObjectIterator<USLSkeletalDataComponent> ObjItr; ObjItr; ++ObjItr)
	//{
	//	// Valid if its parent is a skeletal mesh component
	//	if (Cast<USkeletalMeshComponent>(ObjItr->GetAttachParent()))
	//	{
	//		if (bOverwriteVisualMaskValues)
	//		{
	//			UE_LOG(LogTemp, Warning, TEXT("%s::%d [Skel] Owner=%s;"), *FString(__func__), __LINE__, *ObjItr->GetOwner()->GetName());
	//			
	//			for (auto& Pair : ObjItr->SemanticBonesData)
	//			{
	//				// Check if data is set (it has a semantic class)
	//				if (Pair.Value.IsClassSet())
	//				{
	//					if (UniqueColors.Num() == 0)
	//					{
	//						UE_LOG(LogTemp, Error, TEXT("%s::%d \t Class=%s; No more unique colors, saving as black"),
	//							*FString(__func__), __LINE__, *Pair.Value.Class);
	//						Pair.Value.VisualMask = FColor::Black.ToHex();
	//					}
	//					else
	//					{
	//						const FString ColorStr = UniqueColors.Pop(false);
	//						Pair.Value.VisualMask = ColorStr;
	//						UsedColors++;
	//						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Class=%s; Color=%s; \t\t\t\t [%d/%d]"),
	//							*FString(__func__), __LINE__, *Pair.Value.Class, *ColorStr, UsedColors, UniqueColors.Num());
	//					}

	//					// Add the mask to the map used at runtime as well
	//					if (ObjItr->AllBonesData.Contains(Pair.Key))
	//					{
	//						ObjItr->AllBonesData[Pair.Key].VisualMask = Pair.Value.VisualMask;
	//					}
	//					else
	//					{
	//						// This should not happen, the two mappings should be synced
	//						UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot find bone %s, mappings are not synced.."),
	//							*FString(__func__), __LINE__, *Pair.Key.ToString());
	//					}
	//				}
	//			}
	//		}
	//		else
	//		{
	//			// Not implemented
	//		}
	//	}
	//}
}

// Get already used visual mask colors
TArray<FColor> FSLEdUtils::GetConsumedVisualMaskColors(UWorld* World)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("VisMask");
	TArray<FColor> ConsumedColors;

	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		FString ColHexStr = FSLTagIO::GetValue(*ActItr, TagType, TagKey);
		if (!ColHexStr.IsEmpty())
		{
			ConsumedColors.Add(FColor::FromHex(ColHexStr));
		}
	}

	/* Skeletal mesh actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Get the semantic data component containing the semantics (ids, class names, mask colors) about the bones
		if (UActorComponent* AC = ActItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
		{
			// Load existing visual mask values from the skeletal data
			USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
			for (auto& Pair : SkDC->SemanticBonesData)
			{
				// Check if the bone has a semantic class and and a visual mask
				if (Pair.Value.IsClassSet() && !Pair.Value.VisualMask.IsEmpty())
				{
					ConsumedColors.Add(FColor::FromHex(Pair.Value.VisualMask));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Number of previously stored visual masks=%ld"),
	//	*FString(__func__), __LINE__, ConsumedColors.Num());
	return ConsumedColors;
}

// Add unique mask color
bool FSLEdUtils::AddUniqueVisualMask(AActor* Actor, TArray<FColor>& ConsumedColors, bool bOverwrite)
{
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("VisMask");
	static const int32 MaxTrials = 100;
	static const uint8 BlackColorTolerance = 37;
	static const uint8 WhiteColorTolerance = 23;
	static const uint8 MinColorDistance = 29;

	/* Static mesh actors */
	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Actor))
	{
		// Check if actor already has a visual mask
		FColor PrevMask = FColor::FromHex(FSLTagIO::GetValue(SMA, TagType, TagKey));
		if (PrevMask != FColor::Black)
		{		
			// Explicit overwrite check here, instead when writing to the tag (faster this way)
			if (bOverwrite)
			{
				// Remove soon to be overwritten color from consumed array
				int32 PrevIdx = ConsumedColors.Find(PrevMask);
				if (PrevIdx != INDEX_NONE)
				{
					ConsumedColors.RemoveAt(PrevIdx);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d To be overwritten color for %s is not in the consumed colors array, this should not happen  .."),
						*FString(__func__), __LINE__, *Actor->GetName());
				}

				const FColor MaskColor = NewRandomlyGeneratedUniqueColor(ConsumedColors, MaxTrials, MinColorDistance);
				if (MaskColor == FColor::Black)
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a new visual mask for %s .."),
						*FString(__func__), __LINE__, *Actor->GetName());
					return false;
				}
				else
				{
					const FString MaskColorHex = MaskColor.ToHex();
					//UE_LOG(LogTemp, Warning, TEXT("%s::%d Overwriting with new visual mask, hex=%s, to %s, new total=%ld.."),
					//	*FString(__func__), __LINE__, *MaskColorHex, *Actor->GetName(), ConsumedColors.Num());
					return FSLTagIO::AddKVPair(SMA, TagType, TagKey, MaskColorHex);
				}				
			}
			return false; // Prev color should not be overwritten
		}
		else
		{
			const FColor MaskColor = NewRandomlyGeneratedUniqueColor(ConsumedColors, MaxTrials, MinColorDistance);
			if (MaskColor == FColor::Black)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a new visual mask for %s .."),
					*FString(__func__), __LINE__, *Actor->GetName());
				return false;
			}
			else
			{
				const FString MaskColorHex = MaskColor.ToHex();
				//UE_LOG(LogTemp, Warning, TEXT("%s::%d Overwriting with new visual mask, hex=%s, to %s, new total=%ld.."),
				//	*FString(__func__), __LINE__, *MaskColorHex, *ActItr->GetName(), ConsumedColors.Num());
				return FSLTagIO::AddKVPair(SMA, TagType, TagKey, MaskColorHex);
			}
		}
	}


	/* Skeletal mesh actors */
	if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(Actor))
	{
		// Get the semantic data component containing the semantics (class names mask colors) about the bones
		if (UActorComponent* AC = SkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
		{
			// Iterate visual mask values from the skeletal data
			USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
			for (auto& BoneNameToDataPair : SkDC->SemanticBonesData)
			{
				// Check if bone has a semantic class
				if (!BoneNameToDataPair.Value.IsClassSet())
				{
					continue;
				}

				// Check if the bone has a visual mask
				if (BoneNameToDataPair.Value.HasVisualMask())
				{
					if (bOverwrite)
					{
						// Remove soon to be overwritten color from consumed array
						int32 PrevIdx = ConsumedColors.Find(FColor::FromHex(BoneNameToDataPair.Value.VisualMask));
						if (PrevIdx != INDEX_NONE)
						{
							ConsumedColors.RemoveAt(PrevIdx);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d To be overwritten color for %s-%s is not in the consumed colors array, this should not happen  .."),
								*FString(__func__), __LINE__, *Actor->GetName(), *BoneNameToDataPair.Value.Class);
						}

						const FColor MaskColor = NewRandomlyGeneratedUniqueColor(ConsumedColors, MaxTrials, MinColorDistance);
						if (MaskColor == FColor::Black)
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a new visual mask for %s-%s .."),
								*FString(__func__), __LINE__, *Actor->GetName(), *BoneNameToDataPair.Value.Class);
						}
						else
						{
							const FString MaskColorHex = MaskColor.ToHex();
							//UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Addding new visual mask, hex=%s, to %s --> %s, new total=%ld.."),
							//	*FString(__func__), __LINE__, *BoneNameToDataPair.Value.VisualMask, *ActItr->GetName(), *BoneNameToDataPair.Value.Class, ConsumedColors.Num());
							BoneNameToDataPair.Value.VisualMask = MaskColorHex;
						}

						// Add the mask to the map used at runtime as well
						if (SkDC->AllBonesData.Contains(BoneNameToDataPair.Key))
						{
							SkDC->AllBonesData[BoneNameToDataPair.Key].VisualMask = BoneNameToDataPair.Value.VisualMask;
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
								*FString(__func__), __LINE__, *BoneNameToDataPair.Key.ToString());
						}
					}
				}
				else
				{
					const FColor MaskColor = NewRandomlyGeneratedUniqueColor(ConsumedColors, MaxTrials, MinColorDistance);
					if (MaskColor == FColor::Black)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a new visual mask for %s-%s .."),
							*FString(__func__), __LINE__, *Actor->GetName(), *BoneNameToDataPair.Value.Class);
					}
					else
					{
						const FString MaskColorHex = MaskColor.ToHex();
						//UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Addding new visual mask, hex=%s, to %s --> %s, new total=%ld.."),
						//	*FString(__func__), __LINE__, *BoneNameToDataPair.Value.VisualMask, *ActItr->GetName(), *BoneNameToDataPair.Value.Class, ConsumedColors.Num());
						BoneNameToDataPair.Value.VisualMask = MaskColorHex;
					}

					// Add the mask to the map used at runtime as well
					if (SkDC->AllBonesData.Contains(BoneNameToDataPair.Key))
					{
						SkDC->AllBonesData[BoneNameToDataPair.Key].VisualMask = BoneNameToDataPair.Value.VisualMask;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
							*FString(__func__), __LINE__, *BoneNameToDataPair.Key.ToString());
					}
				}
			}
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			return false;
		}
	}	
	return false; // Not a visual actor
}

// Generate a new unique color avoiding any from the used up array
FColor FSLEdUtils::NewRandomlyGeneratedUniqueColor(TArray<FColor>& ConsumedColors, int32 NumberOfTrials, int32 MinManhattanDistance)
{
	static const uint8 BlackColorDistance = 37;
	static const uint8 WhiteColorDistance = 23;
	for (int32 TrialIdx = 0; TrialIdx < NumberOfTrials; ++TrialIdx)
	{
		// Generate a random color that differs of black
		//FColor RC = FColor::MakeRandomColor(); // Pretty colors, but not many
		FColor RandColor = ColorRandomRGB();

		// Avoid very dark colors, or very bright (
		if (ColorEqual(RandColor, FColor::Black, BlackColorDistance) || ColorEqual(RandColor, FColor::White, WhiteColorDistance))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Got a very dark or bright (reserved) color, hex=%s, trying again.."), *FString(__func__), __LINE__, *RandColor.ToHex());
			continue;
		}

		/* Nested lambda for FindByPredicate*/
		const auto EqualWithToleranceLambda = [RandColor, MinManhattanDistance](const FColor& Item)
		{
			return ColorEqual(RandColor, Item, MinManhattanDistance);
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

