// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class USLIndividualComponent;

// Individual types flags
enum class ESLIndividualFlags : uint32
{
	SLI_None				= 0x00000000u,	///< No flags

	SLI_Base				= 0x00000001u,	///< Has Id and Class

	SLI_Perceivable			= 0x00000002u,	///< Has VisualMask
	SLI_Static				= 0x00000004u,	///< Has VisualMask
	SLI_Skeletal			= 0x00000010u,	///< Has VisualMask
	SLI_Robot				= 0x00000020u,	///< Has VisualMask

	SLI_Constraint			= 0x00000040u,	///< TODO

	SLI_Light				= 0x00000080u,	///< TODO
};

ENUM_CLASS_FLAGS(ESLIndividualFlags);

/* Individual flag helper class */
struct FSLIndiviualType
{
	// Individual type flag
	// TODO check if to use uint32
	ESLIndividualFlags Flag; 

	// Default ctor
	FSLIndiviualType() : Flag(ESLIndividualFlags::SLI_None) {};

	// Init ctor
	FSLIndiviualType(ESLIndividualFlags NewFlag) : Flag(NewFlag) {};

	
	bool IsBase(ESLIndividualFlags InFlag) const
	{
		return (InFlag & ESLIndividualFlags::SLI_Base) != ESLIndividualFlags::SLI_None;
	};

	bool IsPerceivable(ESLIndividualFlags InFlag) const
	{
		return (InFlag & ESLIndividualFlags::SLI_Perceivable) != ESLIndividualFlags::SLI_None;
	};

	FString ToString() const
	{
		switch (Flag)
		{
		case ESLIndividualFlags::SLI_None:			return FString("UnknownIndividual");		break;
		case ESLIndividualFlags::SLI_Base:			return FString("BaseIndividual");			break;
		case ESLIndividualFlags::SLI_Perceivable:	return FString("PerceivableIndividual");	break;
		default:									return FString("DEFAULT");
		}
	};
};

/**
 * Static helpers functions the semantic individual annotation
 */
class USEMLOG_API FSLIndividualUtils
{
public:
	// Get the individual component from the actor (nullptr if it does not exist)
	static USLIndividualComponent* GetIndividualComponent(AActor* Owner);

	// Get the individual object from the actor (nullptr if it does not exist)
	static USLBaseIndividual* GetIndividualObject(AActor* Owner);

	// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
	static FString GetIndividualClassName(USLIndividualComponent* IndividualComponent, bool bDefaultToLabelName = false);

	// Check if actor supports individual components
	static bool CanHaveIndividualComponent(AActor* Actor);

	// Create default individual object depending on the owner type (returns nullptr if failed)
	static UClass* CreateIndividualObject(UObject* Outer, AActor* Owner, class USLBaseIndividual*& OutIndividualObject);

	// Create default individual object depending on the owner type (returns nullptr if failed)
	class USLBaseIndividual* CreateIndividualObject(UObject* Outer, AActor* Owner);

	// Convert individual to the given type
	static bool ConvertIndividualObject(class USLBaseIndividual*& OutIndividualObject, TSubclassOf<class USLBaseIndividual> ConvertToClass);


	/* Id */
	// Write unique id to the actor
	static bool WriteId(USLIndividualComponent* IndividualComponent, bool bOverwrite);

	// Clear unique id of the actor
	static bool ClearId(USLIndividualComponent* IndividualComponent);


	/* Class */
	// Write class name to the actor
	static bool WriteClass(USLIndividualComponent* IndividualComponent, bool bOverwrite);

	// Clear class name of the actor
	static bool ClearClass(USLIndividualComponent* IndividualComponent);


	/* Visual mask */
	// Write unique visual masks for all visual individuals in the world
	static int32 WriteVisualMasks(const TSet<USLIndividualComponent*>& IndividualComponents, bool bOverwrite);

	// Write unique visual masks for visual individuals from the actos in the array
	static int32 WriteVisualMasks(const TSet<USLIndividualComponent*>& IndividualComponentsSelection,
		const TSet<USLIndividualComponent*>& RegisteredIndividualComponents,
		bool bOverwrite);

	// Clear visual mask of the actor
	static bool ClearVisualMask(USLIndividualComponent* IndividualComponent);

private:
	//// Get casted individual from actor (nullptr if failed)
	//template <typename ClassType>
	//static ClassType* GetCastedIndividualObject(AActor* Actor);

	/* Visual mask generation */
	// Add visual mask
	static bool AddVisualMask(class USLPerceivableIndividual* Individual, TArray<FColor>& ConsumedColors, bool bOverwrite);

	// Get all used up visual masks in the world
	static TArray<FColor> GetConsumedVisualMaskColors(const TSet<USLIndividualComponent*>& IndividualComponents);

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

//// Return casted semantic individual object of actor (nullptr if failed)
//template<typename ClassType>
//inline ClassType* FSLIndividualUtils::GetCastedIndividualObject(AActor* Actor)
//{
//	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
//	{
//		return CastChecked<USLIndividualComponent>(AC)->GetCastedIndividualObject<ClassType>();
//	}
//	return nullptr;
//}
