// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/NoExportTypes.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SLVisMaskVisualizer.generated.h"

/**
* Information about the semantic color
*/
struct FSLVisSemanticColorInfo
{
	// Pointer to the semantically annotated UObject
	UObject* Owner;

	// Color in hex
	FString ColorHex;

	// Color
	FColor Color;

	// Semantic class it represents
	FString Class;

	// Unique ID
	FString Id;

	// Checks if all values are set
	bool IsComplete() const { return Owner 
		&& !ColorHex.IsEmpty() 
		&& Color == FColor::FromHex(ColorHex) 
		&& !Class.IsEmpty() 
		&& !Id.IsEmpty(); };

	// Write an output of the struct
	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("Owner=%s; ColorHex=%s; Color=%s; Class=%s; Id=%s; NumPixels=%d"),
			*Owner->GetName(),
			*ColorHex,
			*Color.ToString(),
			*Class,
			*Id);
	}
};


/**
 * 
 */
UCLASS()
class USLVisMaskVisualizer : public UObject
{
	GENERATED_BODY()
public:
	// Ctor
	USLVisMaskVisualizer();

	// Dtor
	~USLVisMaskVisualizer();

	// Init
	void Init();

	// Check if it is init
	bool IsInit() const { return bIsInit; };

	// Returns true if the masks are currently active on the meshes
	bool AreMasksOn() const { return bMaskMaterialsOn; };

	// Apply mask materials
	bool ApplyMaskMaterials();

	// Apply original materials
	bool ApplyOriginalMaterials();

	// Toggle between the mask and original materials
	bool ToggleMaterials();

	// Process the semantic mask image, fix pixel color deviations in image, return entities data
	void ProcessMaskImage(TArray<FColor>& MaskImage, TArray<FSLVisEntitiyData>& OutEntitiesData);

private:
	// Add information about the semantic color (return true if all the fields were filled)
	void AddSemanticColorInfo(const FColor& Color, const FString& ColorHex, UObject* Owner);

	// Compare against the semantic colors, if found switch
	bool ReplaceIfDeviating(FColor& OutColor);

	// Compare the two FColor with a tolerance
	FORCEINLINE bool CompareWithTolerance(const FColor& ColorA, const FColor& ColorB, uint8 Tolerance = 2) const
	{
		return FMath::Abs(ColorA.R - ColorB.R) < Tolerance && FMath::Abs(ColorA.G - ColorB.G) < Tolerance && FMath::Abs(ColorA.B - ColorB.B) < Tolerance;
	}

private:
	// Set when logger is initialized
	bool bIsInit;

	// true if the mask materials are currently are on the meshes
	bool bMaskMaterialsOn;

	// Store the semantic colors in an array (FindByPredicate convenience)
	TArray<FColor> SemanticColors;

	// Semantic color data stored in a map (redundant)
	TMap<FColor, FSLVisEntitiyData> SemanticColorData;

	// Store the information about the semantic color
	TMap<FColor, FSLVisSemanticColorInfo> SemanticColorsInfo;

	// Material used if there is no semantic information on an entity (black)
	UMaterial* DefaultMaskMaterial;

	// Original materials of the meshes
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> OriginalMaterials;

	// Mask materials of the meshes
	UPROPERTY()
	TMap<UMeshComponent*, UMaterialInterface*> MaskMaterials;

	// Meshes with no masks
	TArray<UMeshComponent*> IgnoredMeshes;
};
