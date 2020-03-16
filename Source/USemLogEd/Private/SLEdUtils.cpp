// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)


#include "SLEdUtils.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"

// SL
#include "Editor/SLSemanticMapWriter.h"
#include "Vision/SLVisionCamera.h"
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
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("Id");

	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		/* SMA */
		if (ActItr->IsA(AStaticMeshActor::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
		}

		/* SkMA */
		if (ActItr->IsA(ASkeletalMeshActor::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);

			// Get the semantic data component containing the semantics (class names mask colors) about the bones
			if (UActorComponent* AC = ActItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
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
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
					*FString(__func__), __LINE__, *ActItr->GetName());
			}
		}

		/* Joints */
		if (ActItr->IsA(APhysicsConstraintActor::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
		}

		/* Vision cameras */
		if (ActItr->IsA(ASLVisionCamera::StaticClass()))
		{
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, FSLUuid::NewGuidInBase64Url(), bOverwrite);
		}
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

// Write unique visual masks
void FSLEdUtils::WriteVisualMasks(UWorld* World, bool bOverwrite)
{
	WriteRandomlyGeneratedVisualMasks(World, bOverwrite);
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
	static const FString TagType = TEXT("SemLog");
	static const FString TagKey = TEXT("VisMask");
	
	static const int32 MaxTrials = 100;
	static const uint8 BlackColorTolerance = 37;
	static const uint8 WhiteColorTolerance = 23;
	static const uint8 MinColorDistance = 29;

	// TArray is used instead of TSet, because ot has FindByPredicate
	TArray<FColor> ConsumedColors;

	// Load already used colors to avoid generating similar ones
	if (!bOverwrite)
	{
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
	}

	/* Lambda to return a new unique random color, black if it failed to generate one */
	const auto GenerateANewUniqueColorLambda = [](const int32 MinDist, int32 MaxNumTrials, TArray<FColor>& ConsumedColors)->FString
	{
		for (int32 TrialIdx = 0; TrialIdx < MaxNumTrials; TrialIdx++)
		{
			// Generate a random color that differs of black
			//FColor RC = FColor::MakeRandomColor(); // Pretty colors, but not many
			FColor RandColor = ColorRandomRGB();

			// Avoid very dark colors, or very bright (
			if (ColorEqual(RandColor, FColor::Black, BlackColorTolerance) || ColorEqual(RandColor, FColor::White, WhiteColorTolerance))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Got a very dark or bright (reserved) color, hex=%s, trying again.."), *FString(__func__), __LINE__, *RandColor.ToHex());
				continue;
			}

			/* Nested lambda for FindByPredicate*/
			const auto EqualWithToleranceLambda = [RandColor, MinDist](const FColor& Item)
			{
				return ColorEqual(RandColor, Item, MinDist);
			};

			if (!ConsumedColors.FindByPredicate(EqualWithToleranceLambda))
			{
				ConsumedColors.Emplace(RandColor);
				return RandColor.ToHex();
			}
		}
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not generate a unique color, saving as black.."), *FString(__func__), __LINE__);
		return FColor::Black.ToHex();
	};

	// Write new colors
	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Explicit overwrite check here, instead when writing to the tag (faster this way)
		if (FSLTagIO::HasKey(*ActItr, TagType, TagKey) && bOverwrite)
		{
			if (bOverwrite)
			{
				const FString MaskHex = GenerateANewUniqueColorLambda(MinColorDistance, MaxTrials, ConsumedColors);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Overwriting with new visual mask, hex=%s, to %s, new total=%ld.."),
					*FString(__func__), __LINE__, *MaskHex, *ActItr->GetName(), ConsumedColors.Num());
				FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, MaskHex);
			}
		}
		else
		{
			const FString MaskHex = GenerateANewUniqueColorLambda(MinColorDistance, MaxTrials, ConsumedColors);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Addding new visual mask, hex=%s, to %s, new total=%ld.."),
				*FString(__func__), __LINE__, *MaskHex, *ActItr->GetName(), ConsumedColors.Num());
			FSLTagIO::AddKVPair(*ActItr, TagType, TagKey, MaskHex);
		}
	}

	/* Skeletal mesh actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// Get the semantic data component containing the semantics (class names mask colors) about the bones
		if (UActorComponent* AC = ActItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
		{
			// Load existing visual mask values from the skeletal data
			USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
			for (auto& Pair : SkDC->SemanticBonesData)
			{
				// Check if bone has a semantic class
				if (!Pair.Value.IsClassSet())
				{
					continue;
				}

				// Check if the bone has a visual mask
				if (Pair.Value.HasVisualMask())
				{
					if (bOverwrite)
					{
						Pair.Value.VisualMask = GenerateANewUniqueColorLambda(MinColorDistance, MaxTrials, ConsumedColors);
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Addding new visual mask, hex=%s, to %s --> %s, new total=%ld.."),
							*FString(__func__), __LINE__, *Pair.Value.VisualMask, *ActItr->GetName(), *Pair.Value.Class, ConsumedColors.Num());

						// Add the mask to the map used at runtime as well
						if (SkDC->AllBonesData.Contains(Pair.Key))
						{
							SkDC->AllBonesData[Pair.Key].VisualMask = Pair.Value.VisualMask;
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
								*FString(__func__), __LINE__, *Pair.Key.ToString());
						}
					}
				}
				else
				{
					Pair.Value.VisualMask = GenerateANewUniqueColorLambda(MinColorDistance, MaxTrials, ConsumedColors);
					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Addding new visual mask, hex=%s, to %s --> %s, new total=%ld.."),
						*FString(__func__), __LINE__, *Pair.Value.VisualMask, *ActItr->GetName(), *Pair.Value.Class, ConsumedColors.Num());

					// Add the mask to the map used at runtime as well
					if (SkDC->AllBonesData.Contains(Pair.Key))
					{
						SkDC->AllBonesData[Pair.Key].VisualMask = Pair.Value.VisualMask;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
							*FString(__func__), __LINE__, *Pair.Key.ToString());
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
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