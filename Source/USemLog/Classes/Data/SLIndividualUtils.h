// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Data/SLIndividualComponent.h"

/**
 * Static helpers functions the semantic individual annotation
 */
class USEMLOG_API FSLIndividualUtils
{
public:
	// Add new individual component to actor (return true if component has been created or modified)
	static bool AddNewIndividualComponent(AActor* Actor, bool bOverwrite);

	// Add new visual info component to actor (return true if component has been created or modified)
	static bool AddNewVisualInfoComponent(AActor* Actor, bool bOverwrite);

	// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
	static FString GetIndividualClassName(AActor* Actor, bool bDefaultToLabelName = false);

	// Create default individual object depending on the owner type (returns nullptr if failed)
	static UClass* CreateIndividualObject(UObject* Outer, AActor* Owner, class USLIndividualBase*& OutIndividualObject);

	// Convert individual to the given type
	static bool ConvertIndividualObject(class USLIndividualBase*& OutIndividualObject, TSubclassOf<class USLIndividual> ConvertToClass);

	/* Id */
	// Write unique id to the actor
	static bool WriteId(AActor* Actor, bool bOverwrite);

	// Clear unique id of the actor
	static bool ClearId(AActor* Actor);

	/* Class */
	// Write class name to the actor
	static bool WriteClass(AActor* Actor, bool bOverwrite);

	// Clear class name of the actor
	static bool ClearClass(AActor* Actor);

	/* Visual mask */
	// Write unique visual masks for all visual individuals in the world
	static void WriteVisualMasks(UWorld* World, bool bOverwrite);

	// Write unique visual masks for visual individuals from the actos in the array
	static void WriteVisualMasks(const TArray<AActor*>& Actors, UWorld* World, bool bOverwrite);

	// Clear visual mask of the actor
	static bool ClearVisualMask(AActor* Actor);

private:
	// Check if actor supports individual components
	static bool SupportsIndividualComponents(AActor* Actor);

	// Get casted individual from actor (nullptr if failed)
	template <typename ClassType>
	static ClassType* GetCastedIndividualObject(AActor* Actor);

	/* Visual mask generation */
	// Add visual mask
	static bool AddVisualMask(class USLVisibleIndividual* Individual, TArray<FColor>& ConsumedColors, bool bOverwrite);

	// Get all used up visual masks in the world
	static TArray<FColor> GetConsumedVisualMaskColors(UWorld* World);

	// Create a new unique color by randomization
	static FColor CreateNewUniqueColorRand(TArray<FColor>& ConsumedColors, int32 NumTrials, int32 MinManhattanDist);

	/* Color helpers */
	// Get the manhattan distance between the colors
	FORCEINLINE static int32 GetColorManhattanDistance(const FColor& C1, const FColor& C2)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B);
	}

	// Check if the two colors are equal with a tolerance
	FORCEINLINE static bool AreColorsEqual(const FColor& C1, const FColor& C2, uint8 Tolerance = 0)
	{
		if (Tolerance < 1) { return C1 == C2; }
		return GetColorManhattanDistance(C1, C2) <= Tolerance;
	}

	// Generate a random color
	FORCEINLINE static FColor CreateRandomRGBColor()
	{
		return FColor((uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f));
	}
};


/* Inline functions */

// Return casted semantic individual object of actor (nullptr if failed)
template<typename ClassType>
inline ClassType* FSLIndividualUtils::GetCastedIndividualObject(AActor* Actor)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		return CastChecked<USLIndividualComponent>(AC)->GetCastedIndividualObject<ClassType>();
	}
	return nullptr;
}
