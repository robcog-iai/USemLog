// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Vision/SLVisionPoseableMeshActor.h"

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

	// Default ctor
	FSLVisionLoggerParams();

	// Init ctor
	FSLVisionLoggerParams(
		float InUpdateRate,
		FIntPoint InResolution,
		bool bInIncludeLocally) :
		UpdateRate(InUpdateRate),
		Resolution(InResolution),
		bIncludeLocally(bInIncludeLocally)
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
	TMap<AActor*, FTransform> ActorPoses;

	// Skeletal (poseable) meshes bone transformation
	TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>> SkeletalPoses;

	// Apply transformations, return the frame timestamp
	float ApplyTransformations(
		bool bIncludeMasks,
		TMap<AActor*, AStaticMeshActor*>& MaskClones,
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
		return Timestamp;
	}

	// Clear time and poses
	void Clear() { Timestamp = -1.f; ActorPoses.Empty(); SkeletalPoses.Empty(); };
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
		TMap<AActor*, AStaticMeshActor*>& MaskClones,
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
		TMap<AActor*, AStaticMeshActor*>& MaskClones,
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
struct FSLVisionViewEntitiyData
{
	// Default ctor
	FSLVisionViewEntitiyData() {};

	// Init ctor
	FSLVisionViewEntitiyData(const FString& InId, const FString& InClass) : Id(InId), Class(InClass) {};

	// Unique id of the entity
	FString Id;

	// Class of the entity
	FString Class;

	// The percentage of the entity in the image
	float ImagePercentage;

	// Percentage of the image that is not visible (overlapped + clipped)
	float BlockedPercentage;

	// Percentage of the image that is clipped by the edge
	float OverlappedPercentage;

	// Percentage of the image that is clipped by the edge
	float ClippedPercentage;

	//// Relative transform from the view (virtual camera)
	//FTransform RelativePose;
};

/**
* Semantic skeletal entities data in the view
*/
struct FSLVisionViewSkelBoneData
{
	// Default ctor
	FSLVisionViewSkelBoneData() {};

	// Init ctor
	FSLVisionViewSkelBoneData(const FString& InClass) : Class(InClass) {};

	// Bone class
	FString Class;
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

	// Unique id of the entity
	FString Id;

	// Class of the entity
	FString Class;

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
	TArray<FSLVisionViewEntitiyData> Entities;

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
	float Timestamp;

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
