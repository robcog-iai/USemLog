// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionToolkit.h"
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


void FSLVisionToolkit::VisionToolkitExample(UWorld* World, bool bOverwriteProperties)
{
}

// Randomly generate unique visual masks
void FSLVisionToolkit::RandomlyGenerateVisualMasks(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance)
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
