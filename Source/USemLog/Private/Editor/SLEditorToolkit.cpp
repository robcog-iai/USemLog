// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLEditorToolkit.h"
#include "SLVirtualCameraView.h"
#include "Skeletal/SLSkeletalDataComponent.h"

#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// UUtils
#include "Ids.h"
#include "Tags.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

// Create the semantic map
void FSLEditorToolkit::WriteSemanticMap(UWorld* World, const FString& TaskId, const FString& Filename, ESLOwlSemanticMapTemplate Template)
{
	FSLSemanticMapWriter SemMapWriter;
	SemMapWriter.WriteToFile(World, Template, TaskId, Filename);
}

// Clear tags (all if TagType is empty)
void FSLEditorToolkit::ClearTags(UWorld* World, const FString& TagType, const FString& KeyType)
{
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
#if WITH_EDITOR
		// Apply the changes in the editor world
		if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr))
		{
			if(TagType.IsEmpty())
			{
				EdAct->Tags.Empty();
			}
			else if(!KeyType.IsEmpty())
			{
				FTags::RemoveKeyValuePair(EdAct, TagType, KeyType);
			}
			else
			{
				int32 Idx = FTags::GetTagTypeIndex(EdAct, TagType);
				if(Idx != INDEX_NONE)
				{
					EdAct->Tags.RemoveAt(Idx);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
#endif // WITH_EDITOR
	}
}

// Write class properties to the tags
void FSLEditorToolkit::WriteClassProperties(UWorld* World, bool bOverwrite)
{
	const FString TagType = TEXT("SemLog");
	const FString TagKey = TEXT("Class");

	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, GetClassName(*ActItr), bOverwrite);
	}

	/* Skeletal actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, GetClassName(*ActItr), bOverwrite);
	}

	/* Constraint actors */
	for (TActorIterator<APhysicsConstraintActor> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, GenerateConstraintClassName(*ActItr), bOverwrite);
	}

	/* Vision cameras */
	for (TActorIterator<ASLVirtualCameraView> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, GenerateVisionCameraClassName(*ActItr), bOverwrite);
	}
}

// Write unique id properties
void FSLEditorToolkit::WriteUniqueIdProperties(UWorld* World, bool bOverwrite)
{
	const FString TagType = TEXT("SemLog");
	const FString TagKey = TEXT("Id");

	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);
	}

	/* Skeletal actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
#if WITH_EDITOR
		// Get the skeletal actor from the editor world
		if (AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr))
		{
			FTags::AddKeyValuePair(EdAct, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);

			// Get the semantic data component containing the semantics (class names mask colors) about the bones
			if (UActorComponent* AC = EdAct->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
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
						Pair.Value.Id = FIds::NewGuidInBase64();

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
					else if(Pair.Value.Id.IsEmpty())
					{
						Pair.Value.Id = FIds::NewGuidInBase64();

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
					*FString(__func__), __LINE__, *EdAct->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
#endif //WITH_EDITOR
	}

	/* Constraint actors */
	for (TActorIterator<APhysicsConstraintActor> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);
	}

	/* Vision cameras */
	for (TActorIterator<ASLVirtualCameraView> ActItr(World); ActItr; ++ActItr)
	{
		WritePairToEditorCounterpart(*ActItr, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);
	}
}

// Write unique mask properties
void FSLEditorToolkit::WriteUniqueMaskProperties(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance, bool bRandomGenerator)
{
	if(bRandomGenerator)
	{
		RandomlyGenerateVisualMasks(World, bOverwrite, VisualMaskColorMinDistance);
	}
	else
	{
		IncrementallyGenerateVisualMasks(World, bOverwrite, VisualMaskColorMinDistance);
	}
}

// Tag non-movable objects as static
void FSLEditorToolkit::TagNonMovableEntities(UWorld* World, bool bOverwriteProperties)
{
	const FString TagType = "SemLog";
	const FString KeyType = "Mobility";
	const FString KeyValue = "Static";

	// Check if actor is non movable lambda
	auto IsNonMovableLambda = [](AActor* Actor)->bool
	{
		return false;
	};
	
	
	/* Static mesh actors */
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		// For an actor to be non-movable it has to have physics de-activated, including all the outermosts physics.
		if(FTags::HasKeyValuePair(*ActItr, TagType, KeyType, KeyValue))
		{
			if(!IsNonMovableLambda(*ActItr))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s is movable, tagged as non-movable, removing tag.."),
					*FString(__func__), __LINE__, *ActItr->GetName());
				FTags::RemoveKeyValuePair(*ActItr, TagType, KeyType);
			}
		}
		else
		{
			if(IsNonMovableLambda(*ActItr))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is non-movable, adding tag.."),
					*FString(__func__), __LINE__, *ActItr->GetName());
				FTags::AddKeyValuePair(*ActItr, TagType, KeyType, KeyValue);
			}
		}
	}
}

// Return true if there any duplicates in the class names of the vision cameras
void FSLEditorToolkit::CheckForVisionCameraClassNameDuplicates(UWorld* World)
{
	const FString TagType = "SemLog";
	const FString TagKey = "Class";
	
	TSet<FString> UsedClassNames;
	for(TActorIterator<ASLVirtualCameraView> ActItr(World); ActItr; ++ActItr)
	{
		FString ClassName = "";
#if WITH_EDITOR
		if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr))
		{
			ClassName = FTags::GetValue(EdAct, TagType, TagKey);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find %s in the editor world.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
#endif // WITH_EDITOR
		
		if(ClassName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Vision camera %s has no class name.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
		else if(UsedClassNames.Contains(ClassName))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Class name %s from %s is already used, please rename.."),
				*FString(__func__), __LINE__, *ClassName, *ActItr->GetName());
		}
		
		UsedClassNames.Emplace(ClassName);
	}
}

// Randomly generate unique visual masks
void FSLEditorToolkit::RandomlyGenerateVisualMasks(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance)
{
	const FString TagType = "SemLog";
	const FString KeyType = "VisMask";
	const int32 MaxTrials = 100;

	// TArray is used instead of TSet, because ot has FindByPredicate
	TArray<FColor> ConsumedColors;

	// Load already used colors to avoid generating similar ones
	if(!bOverwrite)
	{
		/* Static mesh actors */
		for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
		{
			FString ColHexStr = FTags::GetValue(*ActItr, TagType, KeyType);
			if(!ColHexStr.IsEmpty())
			{
				ConsumedColors.Add(FColor::FromHex(ColHexStr));
			}
		}
		
		/* Skeletal mesh actors */
		for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
		{
			// Get the semantic data component containing the semantics (ids, class names, mask colors) about the bones
			if(UActorComponent* AC = ActItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
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
			if(ColorEqual(RandColor, FColor::Black, BlackColorTolerance) || ColorEqual(RandColor, FColor::White, WhiteColorTolerance))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Got a very dark or bright (reserved) color, hex=%s, trying again.."), *FString(__func__), __LINE__, *RandColor.ToHex());
				continue;
			}

			/* Nested lambda for FindByPredicate*/
			const auto EqualWithToleranceLambda = [RandColor, MinDist](const FColor& Item)
			{
				return ColorEqual(RandColor, Item, MinDist);
			};

			if(!ConsumedColors.FindByPredicate(EqualWithToleranceLambda))
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
		if(FTags::HasKey(*ActItr, TagType, KeyType))
		{
			if(bOverwrite) // explicit check here since GenerateANewUniqueColorLambda is expensive
			{
#if WITH_EDITOR
				// Apply the changes in the editor world
				if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr))
				{
					const FString MaskHex = GenerateANewUniqueColorLambda(VisualMaskColorMinDistance, MaxTrials, ConsumedColors);
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Overwriting with new visual mask, hex=%s, to %s, new total=%ld.."),
						*FString(__func__), __LINE__, *MaskHex, *ActItr->GetName(), ConsumedColors.Num());
					FTags::AddKeyValuePair(EdAct, TagType, KeyType,MaskHex);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
						*FString(__func__), __LINE__, *ActItr->GetName());
				}
#endif //WITH_EDITOR
			}
		}
		else
		{
#if WITH_EDITOR
			// Apply the changes in the editor world
			if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr))
			{
				const FString MaskHex = GenerateANewUniqueColorLambda(VisualMaskColorMinDistance, MaxTrials, ConsumedColors);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Addding new visual mask, hex=%s, to %s, new total=%ld.."),
					*FString(__func__), __LINE__, *MaskHex, *ActItr->GetName(), ConsumedColors.Num());
				FTags::AddKeyValuePair(EdAct, TagType, KeyType,MaskHex);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
					*FString(__func__), __LINE__, *ActItr->GetName());
			}
#endif //WITH_EDITOR
		}

	}

	/* Skeletal mesh actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
#if WITH_EDITOR
		// Get the skletal actor from the editor world
		if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(*ActItr))
		{
			// Get the semantic data component containing the semantics (class names mask colors) about the bones
			if(UActorComponent* AC = EdAct->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
			{
				// Load existing visual mask values from the skeletal data
				USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
				for (auto& Pair : SkDC->SemanticBonesData)
				{
					// Check if bone has a semantic class
					if(!Pair.Value.IsClassSet())
					{
						continue;
					}

					// Check if the bone has a visual mask
					if (!Pair.Value.VisualMask.IsEmpty())
					{
						if(!bOverwrite)
						{
							continue;
						}

						Pair.Value.VisualMask = GenerateANewUniqueColorLambda(VisualMaskColorMinDistance, MaxTrials, ConsumedColors);
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Addding new visual mask, hex=%s, to %s --> %s, new total=%ld.."),
							*FString(__func__), __LINE__, *Pair.Value.VisualMask, *EdAct->GetName(), *Pair.Value.Class, ConsumedColors.Num());

						// Add the mask to the map used at runtime as well
						if(SkDC->AllBonesData.Contains(Pair.Key))
						{
							SkDC->AllBonesData[Pair.Key].VisualMask = Pair.Value.VisualMask;
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Cannot find bone %s, mappings are not synced.."),
								*FString(__func__), __LINE__, *Pair.Key.ToString());
						}
					}
					else
					{
						Pair.Value.VisualMask = GenerateANewUniqueColorLambda(VisualMaskColorMinDistance, MaxTrials, ConsumedColors);
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Addding new visual mask, hex=%s, to %s --> %s, new total=%ld.."),
							*FString(__func__), __LINE__, *Pair.Value.VisualMask, *EdAct->GetName(), *Pair.Value.Class, ConsumedColors.Num());

						// Add the mask to the map used at runtime as well
						if(SkDC->AllBonesData.Contains(Pair.Key))
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
					*FString(__func__), __LINE__, *EdAct->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
				*FString(__func__), __LINE__, *ActItr->GetName());
		}
#endif //WITH_EDITOR
	}
}

// Incrementally generate unique visual masks
void FSLEditorToolkit::IncrementallyGenerateVisualMasks(UWorld* World, bool bOverwrite,
	int32 VisualMaskColorMinDistance)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d Not implemented.. TODO .."), *FString(__func__), __LINE__);
}

// Get the class name of the actor
FString FSLEditorToolkit::GetClassName(AActor* Actor, bool bDefaultToLabel)
{
	// Get the class name from the asset name
	FString ClassName;
	if(AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Actor))
	{
		if(UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			ClassName = SMC->GetStaticMesh()->GetFullName();
			int32 FindCharPos;
			ClassName.FindLastChar('.', FindCharPos);
			ClassName.RemoveAt(0, FindCharPos + 1);
			if(!ClassName.RemoveFromStart(TEXT("SM_")))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s StaticMesh has no SM_ prefix in its name.."),
				*FString(__func__), __LINE__, *Actor->GetName());
			}
		}
	}
	else if(ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(Actor))
	{
		if(USkeletalMeshComponent* SkMC = SkMA->GetSkeletalMeshComponent())
		{
			ClassName = SkMC->SkeletalMesh->GetFullName();
			int32 FindCharPos;
			ClassName.FindLastChar('.', FindCharPos);
			ClassName.RemoveAt(0, FindCharPos + 1);
			ClassName.RemoveFromStart(TEXT("SK_"));
		}
	}
	else if(bDefaultToLabel)
	{
		ClassName = Actor->GetActorLabel();
	}
	else
	{
		ClassName = "NONE";
	}
	return ClassName;
}

// Generate class name for the constraint actors
FString FSLEditorToolkit::GenerateConstraintClassName(APhysicsConstraintActor* Actor)
{
	FString DefaultValue = "Joint";

	if(UPhysicsConstraintComponent* PCC = Actor->GetConstraintComp())
	{
		// Check if linear
		if(PCC->ConstraintInstance.GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
			PCC->ConstraintInstance.GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
			PCC->ConstraintInstance.GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
		{
			return "Linear"+DefaultValue;
		}
		else if (PCC->ConstraintInstance.GetAngularSwing1Motion() != EAngularConstraintMotion::ACM_Locked ||
			PCC->ConstraintInstance.GetAngularSwing2Motion() != EAngularConstraintMotion::ACM_Locked ||
			PCC->ConstraintInstance.GetAngularTwistMotion() != EAngularConstraintMotion::ACM_Locked)
		{
			return "Revolute" + DefaultValue;
		}
		else
		{
			return "Fixed" + DefaultValue;
		}
	}
	return DefaultValue;
}

// Generate class name for the vision camera
FString FSLEditorToolkit::GenerateVisionCameraClassName(ASLVirtualCameraView* Actor, bool bDefaultToLabel)
{
	const FString DefaultValue = "VisionCamera";
	const FString TagType = "SemLog";
	const FString TagKey = "Class";
	
	if(AActor* AttAct = Actor->GetAttachParentActor())
	{
		// TODO check if skeletal actor, read the bone name for setting the class name
#if WITH_EDITOR
		// Apply the changes in the editor world
		if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(AttAct))
		{
			const FString AttClassName = FTags::GetValue(EdAct, TagType, TagKey);
			return AttClassName + DefaultValue;
		}
#endif // WITH_EDITOR
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.. saving as default value %s .."),
			*FString(__func__), __LINE__, *AttAct->GetName(), *DefaultValue);
		return DefaultValue;
	}

	if (bDefaultToLabel)
	{
		return Actor->GetActorLabel();
	}

	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not use any rule to name the vision camera %s.. saving as default value %s .."),
		*FString(__func__), __LINE__, *Actor->GetName(), *DefaultValue);
	return DefaultValue;
}

// Write tag changes to editor counterpart actor
bool FSLEditorToolkit::WritePairToEditorCounterpart(AActor* Actor, const FString& TagType, const FString& TagKey,
	const FString& TagValue, bool bReplaceExisting)
{
#if WITH_EDITOR
	// Apply the changes in the editor world
	if(AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(Actor))
	{
		return FTags::AddKeyValuePair(EdAct, TagType, TagKey, TagValue, bReplaceExisting);
	}
#endif // WITH_EDITOR
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
		*FString(__func__), __LINE__, *Actor->GetName());
	return false;
}
