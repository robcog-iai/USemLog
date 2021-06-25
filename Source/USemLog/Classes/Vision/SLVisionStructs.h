// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Vision/SLVisionPoseableMeshActor.h"
#include "Vision/SLVirtualCameraView.h"

/**
* View modes
*/
UENUM()
enum class ESLVisionViewMode : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Color					UMETA(DisplayName = "Color"),
	Unlit					UMETA(DisplayName = "Unlit"),
	Mask					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
};


/**
* Vision logger parameters
*/
struct FSLVisionLoggerParams
{
	// Update rate
	float UpdateRate;

	// Resolution
	FIntPoint Resolution;

	// Include locally
	bool bIncludeLocally;

	// Calculate overlaps
	bool bCalculateOverlaps;

	// Make screenshots for calculating overlaps smaller for faster logging
	uint8 OverlapResolutionDivisor;

	// Default ctor
	FSLVisionLoggerParams() {};

	// Init ctor
	FSLVisionLoggerParams(
		float InUpdateRate,
		FIntPoint InResolution,
		bool bInIncludeLocally,
		bool InCalculateOverlaps,
		uint8 InOverlapResolutionDivisor) :
		UpdateRate(InUpdateRate),
		Resolution(InResolution),
		bIncludeLocally(bInIncludeLocally),
		bCalculateOverlaps(InCalculateOverlaps),
		OverlapResolutionDivisor(InOverlapResolutionDivisor)
	{};
};

/**
* Episode frame data
*/
struct FSLVisionFrame
{
	// Frame timestamp
	float Timestamp;

	// Entity poses
	TMap<AStaticMeshActor*, FTransform> ActorPoses;

	// Virtual camera poses
	TMap<ASLVirtualCameraView*, FTransform> VisionCameraPoses;

	// Skeletal (poseable) meshes bone transformation
	TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>> SkeletalPoses;

	// Apply transformations, return the frame timestamp
	float ApplyTransformations(
		bool bIncludeMasks,
		TMap<AStaticMeshActor*, AStaticMeshActor*>& MaskClones,
		TMap<ASLVisionPoseableMeshActor*, ASLVisionPoseableMeshActor*>& SkelMaskClones)
	{
		// Move the static meshes
		for(const auto& Pair : ActorPoses)
		{
			Pair.Key->SetActorTransform(Pair.Value);
			if(bIncludeMasks)
			{
				if(AStaticMeshActor** SMAClone = MaskClones.Find(Pair.Key))
				{
					(*SMAClone)->SetActorTransform(Pair.Value);
				}
			}
		}

		// Move the skeletal(poseable) meshes
		for(const auto& Pair : SkeletalPoses)
		{
			Pair.Key->SetBoneTransforms(Pair.Value);
			if(bIncludeMasks)
			{
				if(ASLVisionPoseableMeshActor** PMAClone = SkelMaskClones.Find(Pair.Key))
				{
					(*PMAClone)->SetBoneTransforms(Pair.Value);
				}
			}
		}

		// Move the virtual cameras
		for (const auto& Pair : VisionCameraPoses)
		{
			Pair.Key->SetActorTransform(Pair.Value);
		}
		return Timestamp;
	}

	// Clear time and poses
	void Clear() { Timestamp = -1.f; ActorPoses.Empty(); SkeletalPoses.Empty(); VisionCameraPoses.Empty(); };
};

/**
* The whole episode data
*/
class FSLVisionEpisode
{
public:
	// Default ctor
	FSLVisionEpisode() : FrameIdx(INDEX_NONE) {};

	// Add a new frame
	int32 AddFrame(const FSLVisionFrame& Frame) { return Frames.Emplace(Frame); };
	
	// Get the active frame in the episode
	int32 GetCurrIndex() const { return FrameIdx; };

	// Get the total number of frames
	int32 GetFramesNum() const { return Frames.Num(); };

	// Move actors to the first frame
	bool SetupFirstFrame(float& OutTimestamp,
		bool bIncludeMasks,
		TMap<AStaticMeshActor*, AStaticMeshActor*>& MaskClones,
		TMap<ASLVisionPoseableMeshActor*, ASLVisionPoseableMeshActor*>& SkelMaskClones)
	{
		FrameIdx = 0;
		if(Frames.IsValidIndex(FrameIdx))
		{
			OutTimestamp = Frames[FrameIdx].ApplyTransformations(bIncludeMasks, MaskClones, SkelMaskClones);
			return true;
		}
		return false;
	}

	// Move actors to the next frame transformations, return false if no more frames are available
	bool SetupNextFrame(float& OutTimestamp,
		bool bIncludeMasks,
		TMap<AStaticMeshActor*, AStaticMeshActor*>& MaskClones,
		TMap<ASLVisionPoseableMeshActor*, ASLVisionPoseableMeshActor*>& SkelMaskClones)
	{
		FrameIdx++;
		if(Frames.IsValidIndex(FrameIdx))
		{
			OutTimestamp = Frames[FrameIdx].ApplyTransformations(bIncludeMasks, MaskClones, SkelMaskClones);
			return true;
		}
		FrameIdx = INDEX_NONE;
		return false;
	}

	// Get first timestamp
	FORCEINLINE float GetFirstTimestamp() const { return Frames.IsValidIndex(0) ? Frames[0].Timestamp : -1.f; };

	// Get last timestamp
	FORCEINLINE float GetLastTimestamp() const { return Frames.Num() > 0 ? Frames.Last().Timestamp : -1.f; };
	
private:
	// All the frames from the episode
	TArray<FSLVisionFrame> Frames;
	
	// Current frame index
	int32 FrameIdx;
};

/**
* Semantic entities data from the view
*/
struct FSLVisionViewEntityData
{
	// Default ctor
	FSLVisionViewEntityData() {};

	// Init ctor
	FSLVisionViewEntityData(const FString& InId, const FString& InClass) : Id(InId), Class(InClass) {};

	// Init ctor with BB
	FSLVisionViewEntityData(const FString& InId, const FString& InClass, const FIntPoint& InMinBB, const FIntPoint& InMaxBB) 
		: Id(InId), Class(InClass), MinBB(InMinBB), MaxBB(InMaxBB) {};

	// Unique id of the entity
	FString Id;

	// Class of the entity
	FString Class;

	// Min bounding box location
	FIntPoint MinBB;

	// Max bounding box location
	FIntPoint MaxBB;

	// The percentage of the entity in the image
	float ImagePercentage = 0.f;

	// Percentage of the image that is occluded by other items
	float OcclusionPercentage = -1.f;

	// True if the image is partially outside of the image
	bool bIsClipped = false;

	//// Percentage of the image that is clipped by the edge
	//float OverlappedPercentage;

	//// Percentage of the image that is clipped by the edge
	//float ClippedPercentage;

	//// Relative transform from the view (virtual camera)
	//FTransform RelativePose;
};

/**
* Semantic skeletal bone data in the view
*/
struct FSLVisionViewSkelBoneData
{
	// Default ctor
	FSLVisionViewSkelBoneData() {};

	// Init ctor
	FSLVisionViewSkelBoneData(const FString& InClass) : Class(InClass) {};

	// Init ctor with BB
	FSLVisionViewSkelBoneData( const FString& InClass, const FIntPoint& InMinBB, const FIntPoint& InMaxBB)
		: Class(InClass), MinBB(InMinBB), MaxBB(InMaxBB) {};

	// Bone class
	FString Class;

	// Min bounding box location of the bone
	FIntPoint MinBB;

	// Max bounding box location of the bone
	FIntPoint MaxBB;

	// The percentage of the entity in the image
	float ImagePercentage = 0.f;

	// Percentage of the image that is occluded by other items
	float OcclusionPercentage = -1.f;

	// True if the image is partially outside of the image
	bool bIsClipped = false;
};

/**
* Semantic skeletal entities data in the view
*/
struct FSLVisionViewSkelData
{
	// Default ctor
	FSLVisionViewSkelData() {};

	// Init ctor
	FSLVisionViewSkelData(const FString& InId, const FString& InClass) : Id(InId), Class(InClass) {};

	// Init ctor with BB
	FSLVisionViewSkelData(const FString& InId, const FString& InClass, const FIntPoint& InMinBB, const FIntPoint& InMaxBB)
		: Id(InId), Class(InClass), MinBB(InMinBB), MaxBB(InMaxBB) {};


	// Calculate the skeletal parameters from the bones
	void CalculateParamsFromBones()
	{
		if(Bones.Num() == 0) { return; }

		MinBB = Bones[0].MinBB;
		MaxBB = Bones[0].MaxBB;
		ImagePercentage = Bones[0].ImagePercentage;

		for(int32 Idx = 1; Idx < Bones.Num(); Idx++)
		{
			if(MinBB.X > Bones[Idx].MinBB.X) { MinBB.X = Bones[Idx].MinBB.X; };
			if(MinBB.Y > Bones[Idx].MinBB.Y) { MinBB.Y = Bones[Idx].MinBB.Y; };
			if(MaxBB.X < Bones[Idx].MaxBB.X) { MaxBB.X = Bones[Idx].MaxBB.X; };
			if(MaxBB.Y < Bones[Idx].MaxBB.Y) { MaxBB.Y = Bones[Idx].MaxBB.Y; };
			ImagePercentage += Bones[Idx].ImagePercentage;
		}
	}

	// Unique id of the entity
	FString Id;

	// Class of the entity
	FString Class;

	// Min bounding box location of the whole skeleton
	FIntPoint MinBB;

	// Max bounding box location of the whole skeleton
	FIntPoint MaxBB;

	// The percentage of the entity in the image
	float ImagePercentage = 0.f;

	// Percentage of the image that is occluded by other items
	float OcclusionPercentage = -1.f;

	// True if the image is partially outside of the image
	bool bIsClipped = false;

	// Bones data
	TArray<FSLVisionViewSkelBoneData> Bones;
};

/**
* Image data
*/
struct FSLVisionImageData
{
	// Default ctor
	FSLVisionImageData() {};
	
	// Init ctor
	FSLVisionImageData(const FString& InType, const TArray<uint8> InData) : Type(InType), Data(InData) {};

	// Image type
	FString Type;

	// Data
	TArray<uint8> Data;
};

/**
* Data from the view
*/
struct FSLVisionViewData
{
	// Default ctor
	FSLVisionViewData() {};

	// Id of the current view
	FString Id;

	// Name of the current view
	FString Class;

	// Data about the entities visible in the view
	TArray<FSLVisionViewEntityData> Entities;

	// Data about the skeletal entities visible in the view
	TArray<FSLVisionViewSkelData> SkelEntities;

	// Array of image data pair, render type name to binary data
	TArray<FSLVisionImageData> Images;

	// Set the initial values
	void Init(const FString& InId, const FString& InClass)
	{
		Id = InId;
		Class = InClass;
	}

	// Clear data
	void Clear()
	{
		Entities.Empty();
		SkelEntities.Empty();
		Images.Empty();
	}
};

/**
* Vision data in the frame
*/
struct FSLVisionFrameData
{
	// Default ctor
	FSLVisionFrameData() {};

	// Timestamp of the frame
	float Timestamp = 0.f;

	// Resolution of the images
	FIntPoint Resolution;

	// Array of the views in the frame
	TArray<FSLVisionViewData> Views;

	// Set the initial values
	void Init(float InTimestamp, const FIntPoint& InResolution)
	{
		Timestamp = InTimestamp;
		Resolution = InResolution;
	}

	// Clear data
	void Clear()
	{
		Views.Empty();
	}
};
