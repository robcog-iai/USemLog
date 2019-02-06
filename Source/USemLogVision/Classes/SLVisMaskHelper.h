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
public:
	// Default constructor
	FSLVisMaskHelper();

	// Init material constructor
	FSLVisMaskHelper(UMaterialInterface* InDefaultMaskMaterial);

	// Cache of the semantic original materials
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> SemOrigMaterials;

	// Cache of the semantic mask values
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> SemMaskMaterials;

	// Cache of the non semantic original materials
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> NonSemOrigMaterials;

	// Cache of the non semantic mask materials
	TMap<UMeshComponent*, TArray<UMaterialInterface*>> NonSemMaskMaterials;

	// Init
	void Init(UWorld* World);

	// Check if it is init
	bool IsInit() const { return bIsInit; };

private:
	// Set when logger is initialized
	bool bIsInit;

	// Material used if there is no semantic information on an entity (black)
	UMaterialInterface* DefaultMaskMaterial;
};