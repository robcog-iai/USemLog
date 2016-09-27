// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/SkeletalMeshActor.h"
#include "SLItem.h"

/**
 * Class exporting raw data during gameplay
 */
class SEMLOG_API FSLRawDataExporter
{
public:
	// Constructor
	FSLRawDataExporter(
		const float DistThreshSqr,
		const TArray<ASLItem*>& DynamicItems,
		const TArray<ASLItem*>& StaticItems,
		const TMap<ASkeletalMeshActor*, FString>& SkelActPtrToUniqNameMap,
		const TPair<USceneComponent*, FString> CamToUniqName,
		const FString Path);

	// Destructor
	~FSLRawDataExporter();

	// Log step
	void Update(const float Timestamp);

	// Structure of skeletal mesh comp with its previous pose
	struct SkelRawStruct
	{
		SkelRawStruct(ASkeletalMeshActor* SkMComp, const FString UniqName) :
			SkelMeshComp(SkMComp),
			UniqueName(UniqName),
			PrevLoc(FVector(0.0f)),
			PrevRot(FRotator(0.0f)) {};
		ASkeletalMeshActor* SkelMeshComp;
		FString UniqueName;
		FVector PrevLoc;
		FRotator PrevRot;
	};

	// Structure of the items and their prev pose
	struct ItemRawStruct
	{
		ItemRawStruct(ASLItem* It) :
			Item(It),
			PrevLoc(FVector(0.0f)),
			PrevRot(FRotator(0.0f)) {};
		ASLItem* Item;
		FVector PrevLoc;
		FRotator PrevRot;
	};
	
private:
	// Init items to log from the level
	void InitItemsToLog(
		const TArray<ASLItem*>& DynamicItems,
		const TArray<ASLItem*>& StaticItems,
		const TMap<ASkeletalMeshActor*, FString>& SkelActPtrToUniqNameMap);

	// Create Json object with a 3d location
	TSharedPtr<FJsonObject> CreateLocationJsonObject(const FVector Location);

	// Create Json object with a 3d rotation as quaternion 
	TSharedPtr<FJsonObject> CreateRotationJsonObject(const FQuat Rotation);

	// Create Json object with name location and rotation
	TSharedPtr<FJsonObject> CreateNameLocRotJsonObject(
		const FString Name, const FVector Location, const FQuat Rotation);

	// Distance threshold (squared) for raw data logging
	float DistanceThresholdSquared;

	// File handle to append raw data
	TSharedPtr<IFileHandle> RawFileHandle;

	// Array of skeletal meshes with prev position and orientation
	TArray<SkelRawStruct> SkelActStructArr;

	// Array of static meshes with prev position and orientation
	TArray<ItemRawStruct> DynamicItemsStructArr;

	// User camera to unique name
	TPair<USceneComponent*, FString> CameraToUniqueName;

	// Array of the static items
	TArray<ASLItem*> StaticItemsArr;
	
	// Camera previous location
	FVector CameraPrevLoc;
};

