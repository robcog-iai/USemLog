// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLVisionStructs.h"

/**
* Image pixel color related data, convenient mapping of mask colors to their semantic data
*/
struct FSLVisionEntityClassAndId
{
	// Default ctor
	FSLVisionEntityClassAndId(){};
	
	// Init ctor
	FSLVisionEntityClassAndId(const FString& InClass, const FString& InId) : Class(InClass), Id(InId) {};

	// Class name
	FString Class;

	// Id
	FString Id;
};

/**
* Image pixel color related data, convenient mapping of mask colors to their semantic data
*/
struct FSLVisionSkelEntityClassAndId
{
	// Default ctor
	FSLVisionSkelEntityClassAndId() {};

	// Init ctor
	FSLVisionSkelEntityClassAndId(const FString& InClass, const FString& InId, const FString& InBoneClass) : 
		Class(InClass), Id(InId), BoneClass(InBoneClass) {};

	// Skeletal entity class name
	FString Class;

	// Id
	FString Id;

	// Bone class
	FString BoneClass;
};


/**
* Image pixel color related data
*/
struct FSLVisionImageColorData
{
	// Default constructor
	FSLVisionImageColorData() : Num(0) {};

	// Init ctor
	FSLVisionImageColorData(int32 InNum, const FIntPoint& InMinBB, const FIntPoint& InMaxBB) : Num(InNum), MinBB(InMinBB), MaxBB(InMaxBB) {};

	// Number of pixels in image
	int32 Num;

	// Min bounding box value in image
	FIntPoint MinBB;

	// Max bounding box value in image
	FIntPoint MaxBB;
};

/**
 * 
 */
class FSLVisionImageHandler
{
public:
	// Ctor
	FSLVisionImageHandler();

	// Load the color to entities mapping
	void Init();

	// Get entities from mask image
	void GetEntities(const TArray<FColor>& InMaskBitmap, int32 ImgWidth, int32 ImgHeight, FSLVisionViewData& OutViewData) const;

private:
	// Init flag
	bool bIsInit;

	// Color to entity data
	TMap<FColor, FSLVisionEntityClassAndId> ColorToEntityData;

	// Color to skeletal entity data
	TMap<FColor, FSLVisionSkelEntityClassAndId> ColorToSKelEntityData;
};
