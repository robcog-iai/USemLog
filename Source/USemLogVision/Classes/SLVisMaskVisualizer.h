// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SLVisMaskVisualizer.generated.h"

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
	bool AreMasksOn() const { return bAreMaskMaterialsOn; };

	// Apply mask materials
	void ApplyMaskMaterials();

	// Apply original materials
	void ApplyOriginalMaterials();

	// Toggle between the mask and original materials
	void Toggle();

private:
	// Set when logger is initialized
	bool bIsInit;

	// true if the mask materials are currently are on the meshes
	bool bAreMaskMaterialsOn;

	// Material used if there is no semantic information on an entity (black)
	UMaterial* DefaultMaskMaterial;

	// Original materials of the meshes
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> OriginalMaterials;

	// Mask materials of the meshes
	UPROPERTY()
	TMap<UMeshComponent*, UMaterialInterface*> MaskMaterials;

	// Meshes with no chosen masks
	TArray<UMeshComponent*> IgnoredMeshes;
};
