// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SLStructs.h"
#include "SLSkeletalMeshActor.h"

/**
 * Abstract class for world state data writer
 */
class ISLWorldStateWriter
{
public:
	// Constructor
	ISLWorldStateWriter(float InDistanceStepSize, float InRotationStepSize) :
		DistanceStepSizeSquared(InDistanceStepSize*InDistanceStepSize),
		RotationStepSize(InRotationStepSize) {};

	// Virtual destructor
	virtual ~ISLWorldStateWriter() {};

	// Write data (it also removes invalid items from the array -- e.g. deleted ones)
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) = 0;

protected:
	// Location step size (log items that moved at least this distance since the last log)
	float DistanceStepSizeSquared;

	// Rotation step size (log items that rotated at least this value since the last log)
	float RotationStepSize;
};
