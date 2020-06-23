// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLIndividualVisualInfoAssets.generated.h"

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
class USEMLOG_API USLIndividualVisualInfoAssets : public UDataAsset
{
	GENERATED_BODY()
	
public:

	/* Meshes */
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* LineStartMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* LineMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* LineEndMesh;

	/* Materials */
	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* TextMaterialTranslucent;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* TextMaterialOpaque;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* LineMaterial;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* LineMaterialTranslucent;

	/* Fonts */
	UPROPERTY(EditAnywhere, Category = Font)
	UFont* TextFont;
};
