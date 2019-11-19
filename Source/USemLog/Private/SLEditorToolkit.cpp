// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEditorToolkit.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"

// UUtils
#include "Ids.h"
#include "Tags.h"
#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR


// Ctor
FSLEditorToolkit::FSLEditorToolkit()
{
}

// Dtor
FSLEditorToolkit::~FSLEditorToolkit()
{
}

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
		WriteKVToEditorCounterpart(*ActItr, TagType, TagKey, GetClassName(*ActItr), bOverwrite);
	}

	/* Skeletal actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		WriteKVToEditorCounterpart(*ActItr, TagType, TagKey, GetClassName(*ActItr), bOverwrite);
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
		WriteKVToEditorCounterpart(*ActItr, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);
	}

	/* Skeletal actors */
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		WriteKVToEditorCounterpart(*ActItr, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);
	}

	/* Constraint actors */
	for (TActorIterator<APhysicsConstraintActor> ActItr(World); ActItr; ++ActItr)
	{
		WriteKVToEditorCounterpart(*ActItr, TagType, TagKey, FIds::NewGuidInBase64(), bOverwrite);
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

// Randomly generate unique visual masks
void FSLEditorToolkit::RandomlyGenerateVisualMasks(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance)
{
	const FString TagType = "SemLog";
	const FString KeyType = "VisMask";
	const int32 MaxTrials = 100;

	// Array has FindByPredicate, TSet does not
	TArray<FColor> ConsumedColors;

	// Cache used colors
	if(!bOverwrite)
	{
		/* Static mesh actors */
		for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
		{
			FString ColHexStr = FTags::GetValue(*ActItr, TagType, KeyType);
			if(!ColHexStr.IsEmpty())
			{
				bool bIsAlreadyInSet = false;
				FColor C = FColor::FromHex(ColHexStr);
				ConsumedColors.Add(C);
			}
		}
		
		/* Skeletal mesh actors */
		// TODO Skeletal
		for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
		{
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
			FColor RC = ColorRandomRGB();

			// Avoid very dark colors
			if(ColorEqual(RC, FColor::Black, 30))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Got a very dark color, hex=%s, trying again.."), *FString(__func__), __LINE__, *RC.ToHex());
				continue;
			}

			/* Nested lambda for FindByPredicate*/
			const auto EqualWithToleranceLambda = [RC, MinDist](const FColor& Item)
			{
				return ColorEqual(RC, Item, MinDist);
			};

			if(!ConsumedColors.FindByPredicate(EqualWithToleranceLambda))
			{
				ConsumedColors.Emplace(RC);
				return RC.ToHex();
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
	// TODO Skeletal
	for (TActorIterator<ASkeletalMeshActor> ActItr(World); ActItr; ++ActItr)
	{
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

// Write to editor counterpart
bool FSLEditorToolkit::WriteKVToEditorCounterpart(AActor* Actor, const FString& TagType, const FString& TagKey,
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
