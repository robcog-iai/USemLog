// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/NoExportTypes.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SLVisMaskHandler.generated.h"

/**
* Structure holding a skeletal mesh and its bone mapped materials
*/
USTRUCT()
struct FSLVisSkelMeshMasks
{
	GENERATED_BODY()

	// The mesh component
	//UPROPERTY()
	UMeshComponent* Mesh;

	// Slot index to material map
	UPROPERTY() // Needed otherwise material get GC'd or marked dirty ?
	TMap<int32, UMaterialInterface*> MaskMaterials;

	// Material used if there is no semantic information on an entity (black)
	//UPROPERTY()
	UMaterialInterface* DefaultMaskMaterial;

	// Default constructor
	FSLVisSkelMeshMasks() {};

	// Init constructor
	FSLVisSkelMeshMasks(UMeshComponent* InMesh, UMaterialInterface* InDefaultMaskMaterial,
		const TMap<int32, UMaterialInterface*>& InMaterials = TMap<int32, UMaterialInterface*>{})
		: Mesh(InMesh), MaskMaterials(InMaterials), DefaultMaskMaterial(InDefaultMaskMaterial) {};

	// Apply the materials to the mesh component
	void ApplyMaterials()
	{
		if (Mesh && DefaultMaskMaterial)
		{
			for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
			{
				if (MaskMaterials.Contains(Idx))
				{
					Mesh->SetMaterial(Idx, MaskMaterials[Idx]);
				}
				else
				{
					Mesh->SetMaterial(Idx, DefaultMaskMaterial);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Mesh or DefaultMaskMaterial is not valid.."), *FString(__func__), __LINE__);
		}
	};
};

/**
 * Helper class for toggling between the mask and original materials
 */
UCLASS()
class USLVisMaskHandler : public UObject
{
	GENERATED_BODY()
public:
	// Ctor
	USLVisMaskHandler();

	// Dtor
	~USLVisMaskHandler();

	// Init
	void Init(int32 InNumOfPixels);

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
	void ProcessMaskImage(TArray<FColor>& MaskImage, const FTransform& ViewWorldTransform, FSLVisViewData& OutViewData);

private:
	// Save the original color materials of the static meshes, and create a for each mesh a mask material
	void SetupStaticMeshes();

	// Save the original color materials of the skeletal meshes, and create a mask material for each semantically annotated bone
	void SetupSkeletalMeshes();

	// Store the information about the semantic color
	void AddSemanticData(const FColor& Color, const FString& ColorHex, const TArray<FName>& Tags, UObject* Self = nullptr);

	// Store the information about the skeletal semantic color
	void AddSkelSemanticData(const FString& OwnerId,
		const FString& OwnerClass,
		const FColor& Color,
		const FString& ColorHex,
		const FString& BoneClass,
		const FTransform& WorldTransform);

	// Compare against the semantic colors, if found, switch
	bool RestoreIfCloseToAMaskColor(FColor& OutColor);

	// Remove entities that have a very small relative number of pixels in the image and set the avg distance in the scene
	void SetOutputData(TMap<FColor, FSLVisEntitiyData>& EntitiesInImage,
		TMap<FColor, FSLVisBoneData>& BonesInImage,
		FSLVisViewData& OutViewData);

	// Compare the two FColor with a tolerance
	FORCEINLINE bool AlmostEqual(const FColor& ColorA, const FColor& ColorB, uint8 Tolerance = 2) const
	{
		return FMath::Abs(ColorA.R - ColorB.R) <= Tolerance 
			&& FMath::Abs(ColorA.G - ColorB.G) <= Tolerance 
			&& FMath::Abs(ColorA.B - ColorB.B) <= Tolerance;
	}

private:
	// Set when logger is initialized
	bool bIsInit;

	// true if the mask materials are currently are on the meshes
	bool bMaskMaterialsOn;

	// Store the semantic colors in an array (FindByPredicate convenience)
	TArray<FColor> MaskColors;

	// Semantic color data stored in a map (bit redundant with the color array)
	TMap<FColor, FSLVisEntitiyData> EntitiesMasks;

	// Used for quick access to the bone data (all this is also stored in the skeletal array data)
	TMap<FColor, FSLVisBoneData> BonesMasks;
	
	// Material used if there is no semantic information on an entity (black)
	UMaterial* DefaultMaskMaterial;

	// Original materials of the meshes
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> OriginalMaterials;

	// Mask materials of the meshes
	UPROPERTY()
	TMap<UMeshComponent*, UMaterialInterface*> MaskMaterials;

	// Mask materials of the skeletal meshes
	UPROPERTY()
	TArray<FSLVisSkelMeshMasks> SkelMaskMaterials;

	// Meshes with no masks
	TArray<UMeshComponent*> IgnoredMeshes;

	// Total num of pixels in the rendered image, used for calculating the percentage in order to ignore barely visible entities
	int32 TotalNumOfPixelsInImg;

	/* Constants */
	// Ignore percentage (normalized) for the relative number of pixels in an image
	constexpr static const float IgnorePercentage = 0.005;

	// Ignore entities with less than 100 pixels;
	constexpr static const int32 AbsoluteMinNumOfPixels = 150;
};
