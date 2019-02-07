// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"

/**
 * Mask visualizer helper
 */
struct FSLVisMaskHelper
{
	// Cache of the original materials
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> MeshesOrigMaterials;

	// Cache of the semantic mask values
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> MeshesMaskMaterials;

	// Cache of the mesh components that are not semantically annotated (will be colored black)
	TArray<UMeshComponent*> UnknownMeshes;

	// Default constructor
	FSLVisMaskHelper();

	// Init material constructor
	FSLVisMaskHelper(UMaterial* InDefaultMaskMaterial);

	// Init
	void Init(UObject* InParent);

	// Check if it is init
	bool IsInit() const { return bIsInit; };

	// Apply mask materials
	bool ApplyMaskMaterials();

	// Apply original materials
	bool ApplyOriginalMaterials();

private:
	// Set when logger is initialized
	bool bIsInit;

	// Material used if there is no semantic information on an entity (black)
	UMaterial* DefaultMaskMaterial;
};