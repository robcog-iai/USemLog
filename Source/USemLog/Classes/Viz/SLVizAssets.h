// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLVizAssets.generated.h"

// Forward declarations
class UStaticMesh;
class UMaterial;

/**
 * Asset container for visual marker representation
 */
UCLASS()
class USEMLOG_API USLVizAssets : public UDataAsset
{
	GENERATED_BODY()
	
public:

	/* Meshes */
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshBox;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshSphere;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshCylinder;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshArrow;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshArrowX;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshArrowY;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshArrowZ;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* MeshAxis;


	/* Materials */
	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterial* MaterialLit;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterial* MaterialUnlit;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterial* MaterialInvisible;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterial* MaterialHighlightAdditive;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterial* MaterialHighlightTranslucent;
};
