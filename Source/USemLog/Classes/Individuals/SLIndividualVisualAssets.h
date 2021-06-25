// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLIndividualVisualAssets.generated.h"

// Forward declarations
class UStaticMesh;
class UMaterial;
class UMaterialInterface;
class UMaterialInstance;
class UFont;

/**
 * Asset container for individual visual info representation
 */
UCLASS()
class USEMLOG_API USLIndividualVisualAssets : public UDataAsset
{
	GENERATED_BODY()
	
public:

	/* Meshes */
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* SplineMesh;

	/* Materials */
	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* TextMaterialTranslucent;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* TextMaterialOpaque;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* SplineMaterial;

	/* Fonts */
	UPROPERTY(EditAnywhere, Category = Font)
	UFont* TextFont;
};
