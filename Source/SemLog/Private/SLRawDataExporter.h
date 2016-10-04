// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/SkeletalMeshActor.h"

/**
 * Class exporting raw data during gameplay
 */
class SEMLOG_API FSLRawDataExporter
{
public:
	// Constructor
	FSLRawDataExporter(const float DistThresh, const FString Path);

	// Destructor
	~FSLRawDataExporter();
	
	// Initial write (static actors included), store dynamic actors for future references
	void WriteInit(
		const TMap<AActor*, FString>& ActToUniqueName,
		const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo,
		const float Timestamp);

	// Raw logger update
	void Update(const float Timestamp);

	// Raw datastructure with the dynamic actors, previous position and unique name
	struct RawExpActStruct
	{
		RawExpActStruct(AActor* Act, const FString UName) :
			Actor(Act),
			UniqueName(UName),
			PrevLoc(FVector(0.0f)){};
		AActor* Actor;
		const FString UniqueName;
		FVector PrevLoc;
	};
	
private:
	// Create Json object with a 3d location
	FORCEINLINE TSharedPtr<FJsonObject> CreateLocationJsonObject(const FVector Location);

	// Create Json object with a 3d rotation as quaternion 
	FORCEINLINE TSharedPtr<FJsonObject> CreateRotationJsonObject(const FQuat Rotation);

	// Create Json object with name location and rotation
	FORCEINLINE TSharedPtr<FJsonObject> CreateNameLocRotJsonObject(
		const FString Name, const FVector Location, const FQuat Rotation);

	// Add the actors raw data to the json array
	FORCEINLINE void AddActorToJsonArray(RawExpActStruct& RawActStruct,
		TArray<TSharedPtr<FJsonValue>>& JsonArray);

	// Distance threshold (squared) for raw data logging
	float DistanceThresholdSquared;

	// File handle to append raw data
	TSharedPtr<IFileHandle> RawFileHandle;

	// Array of the raw exporter datastructure
	TArray<RawExpActStruct> RawExpActArr;
};

