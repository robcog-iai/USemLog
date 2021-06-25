// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class USLIndividualComponent;
class USLBaseIndividual;
class USLSkeletalDataAsset;
class ASLIndividualManager;

//// Individual types flags
//enum class ESLIndividualFlags : uint32
//{
//	SLI_None				= 0x00000000u,	///< No flags
//
//	SLI_Base				= 0x00000001u,	///< Has Id and Class
//
//	SLI_Visible			= 0x00000002u,	///< Has VisualMask
//	SLI_Static				= 0x00000004u,	///< Has VisualMask
//	SLI_Skeletal			= 0x00000010u,	///< Has VisualMask
//	SLI_Robot				= 0x00000020u,	///< Has VisualMask
//
//	SLI_Constraint			= 0x00000040u,	///< TODO
//
//	SLI_Light				= 0x00000080u,	///< TODO
//};
//
//ENUM_CLASS_FLAGS(ESLIndividualFlags);
//
///* Individual flag helper class */
//struct FSLIndiviualType
//{
//	// Individual type flag
//	// TODO check if to use uint32
//	ESLIndividualFlags Flag; 
//
//	// Default ctor
//	FSLIndiviualType() : Flag(ESLIndividualFlags::SLI_None) {};
//
//	// Init ctor
//	FSLIndiviualType(ESLIndividualFlags NewFlag) : Flag(NewFlag) {};
//
//	
//	bool IsBase(ESLIndividualFlags InFlag) const
//	{
//		return (InFlag & ESLIndividualFlags::SLI_Base) != ESLIndividualFlags::SLI_None;
//	};
//
//	bool IsVisible(ESLIndividualFlags InFlag) const
//	{
//		return (InFlag & ESLIndividualFlags::SLI_Visible) != ESLIndividualFlags::SLI_None;
//	};
//
//	FString ToString() const
//	{
//		switch (Flag)
//		{
//		case ESLIndividualFlags::SLI_None:			return FString("UnknownIndividual");		break;
//		case ESLIndividualFlags::SLI_Base:			return FString("BaseIndividual");			break;
//		case ESLIndividualFlags::SLI_Visible:	return FString("VisibleIndividual");	break;
//		default:									return FString("DEFAULT");
//		}
//	};
//};

/**
 * Static helpers functions the semantic individual annotation
 */
class USEMLOG_API FSLIndividualUtils
{
public:
	/* Individuals */
	static ASLIndividualManager* GetOrCreateNewIndividualManager(UWorld* World, bool bCreateNew = true);
	static int32 CreateIndividualComponents(UWorld* World);
	static int32 CreateIndividualComponents(const TArray<AActor*>& Actors);
	static int32 ClearIndividualComponents(UWorld* World);
	static int32 ClearIndividualComponents(const TArray<AActor*>& Actors);
	static int32 InitIndividualComponents(UWorld* World, bool bReset);
	static int32 InitIndividualComponents(const TArray<AActor*>& Actors, bool bReset);
	static int32 LoadIndividualComponents(UWorld* World, bool bReset, bool bTryImport);
	static int32 LoadIndividualComponents(const TArray<AActor*>& Actors, bool bReset, bool bTryImport);
	static int32 ConnectIndividualComponents(UWorld* World);
	static int32 ConnectIndividualComponents(const TArray<AActor*>& Actors);

	/* Functionalities */
	static int32 ToggleVisualMaskVisibility(UWorld* World, bool bIncludeChildren);
	static int32 ToggleVisualMaskVisibility(const TArray<AActor*>& Actors, bool bIncludeChildren);
	
	/* Values */
	/* Ids */
	static int32 WriteIds(UWorld* World, bool bOverwrite);
	static int32 WriteIds(const TArray<AActor*>& Actors, bool bOverwrite);
	static int32 ClearIds(UWorld* World);
	static int32 ClearIds(const TArray<AActor*>& Actors);

	/* Classes */
	static int32 WriteClasses(UWorld* World, bool bOverwrite);
	static int32 WriteClasses(const TArray<AActor*>& Actors, bool bOverwrite);
	static int32 ClearClasses(UWorld* World);
	static int32 ClearClasses(const TArray<AActor*>& Actors);


	/* Visual mask */
	static int32 WriteUniqueVisualMasks(UWorld* World, bool bOverwrite);
	static int32 WriteUniqueVisualMasks(const TArray<AActor*>& Actors, bool bOverwrite);
	static int32 ClearVisualMasks(UWorld* World);
	static int32 ClearVisualMasks(const TArray<AActor*>& Actors);

	/* Export/import values */
	static int32 ExportValues(UWorld* World, bool bOverwrite);
	static int32 ExportValues(const TArray<AActor*>& Actors, bool bOverwrite);
	static int32 ImportValues(UWorld* World, bool bOverwrite);
	static int32 ImportValues(const TArray<AActor*>& Actors, bool bOverwrite);
	static int32 ClearExportedValues(UWorld* World);
	static int32 ClearExportedValues(const TArray<AActor*>& Actors);

	/* Misc */
	// Get the individual component from the actor (nullptr if it does not exist)
	static USLIndividualComponent* GetIndividualComponent(AActor* Owner);

	// Get the individual object from the actor (nullptr if it does not exist)
	static USLBaseIndividual* GetIndividualObject(AActor* Owner);

	// Create default individual object depending on the owner type (returns nullptr if failed)
	static USLBaseIndividual* CreateIndividualObject(UObject* Outer, AActor* Owner);

	// Convert individual to the given type
	static bool ConvertIndividualObject(class USLBaseIndividual*& OutIndividualObject, TSubclassOf<class USLBaseIndividual> ConvertToClass);

	// Generate a new bson oid as string, empty string if fails
	static FString NewOIdAsString();

	// Find the skeletal data asset for the individual
	static USLSkeletalDataAsset* FindSkeletalDataAsset(AActor* Owner);

private:
	/* Individuals Private */
	static USLIndividualComponent* AddNewIndividualComponent(AActor* Actor, bool bTryInitAndLoad = false);
	static bool CanHaveIndividualComponent(AActor* Actor);
	static bool HasIndividualComponent(AActor* Actor);
	static bool ClearIndividualComponent(AActor* Actor);
	static bool InitIndividualComponent(AActor* Actor, bool bReset);
	static bool LoadIndividualComponent(AActor* Actor, bool bReset, bool bTryImport);
	static bool ConnectIndividualComponent(AActor* Actor);

	/* Individuals functionalities Private */
	static bool ToggleVisualMaskVisibility(AActor* Actor, bool bIncludeChildren);

	/* Individuals values Private */
	/* Ids */
	static bool WriteId(AActor* Actor, bool bOverwrite);
	static bool ClearId(AActor* Actor);

	/* Class */
	static bool WriteClass(AActor* Actor, bool bOverwrite);
	static bool ClearClass(AActor* Actor);

	/* Visual Mask */
	static bool WriteUniqueVisualMask(AActor* Actor, TArray<FColor>& ConsumedColors, bool bOverwrite);
	static bool ClearVisualMask(AActor* Actor);
	
	/* Visual Mask  Helpers */
	static TArray<FColor> GetAllConsumedVisualMaskColorsInWorld(UWorld* World);
	static FColor GenerateRandomUniqueColor(TArray<FColor>& ConsumedColors, int32 NumTrials, int32 MinManhattanDist);

	/* Color helpers */
	// Get the manhattan distance between the colors
	FORCEINLINE static int32 GetColorManhattanDistance(const FColor& C1, const FColor& C2)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B);
	}

	// Check if the two colors are equal with a tolerance
	FORCEINLINE static bool AreColorsAlmostEqual(const FColor& C1, const FColor& C2, uint8 Tolerance = 0)
	{
		if (Tolerance < 1) { return C1 == C2; }
		return GetColorManhattanDistance(C1, C2) <= Tolerance;
	}

	// Generate a random color
	FORCEINLINE static FColor CreateRandomRGBColor()
	{
		return FColor((uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f));
	}
	

	/* Import/export values*/
	static bool ExportValues(AActor* Actor, bool bOverwrite);
	static bool ImportValues(AActor* Actor, bool bOverwrite);
	static bool ClearExportedValues(AActor* Actor);


	/* Legacy */
	//// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
	//static FString GetIndividualClassName(USLIndividualComponent* SiblingIndividualComponent, bool bDefaultToLabelName = false);


};
