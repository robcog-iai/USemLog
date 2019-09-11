// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLogVision.h"
#include "UObject/NoExportTypes.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SLVisLiveViewMaskHandler.generated.h"

/**
* Structure holding a skeletal mesh and its bone mapped materials
*/
USTRUCT()
struct FSLVisSkelMasks
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
	FSLVisSkelMasks() {};

	// Init constructor
	FSLVisSkelMasks(UMeshComponent* InMesh, UMaterialInterface* InDefaultMaskMaterial,
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
 * Helper class for toggling between the mask and original materials during live view
 */
UCLASS()
class USLVisLiveViewMaskHandler : public UObject
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLVisLiveViewMaskHandler();

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

private:
	// Save the original color materials of the static meshes, and create a for each mesh a mask material
	void SetupStaticMeshes();

	// Save the original color materials of the skeletal meshes, and create a mask material for each semantically annotated bone
	void SetupSkeletalMeshes();

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
	UPROPERTY() // Avoid GC
	TMap<UMeshComponent*, UMaterialInterface*> MaskMaterials;

	// Mask materials of the skeletal meshes
	UPROPERTY() // Avoid GC
	TArray<FSLVisSkelMasks> SkelMaskMaterials;

	// Meshes with no masks
	TArray<UMeshComponent*> IgnoredMeshes;

};
