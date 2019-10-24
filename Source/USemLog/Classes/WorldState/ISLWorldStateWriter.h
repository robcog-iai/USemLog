// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SLStructs.h"
#include "SLSkeletalDataComponent.h"
#include "SLGazeDataHandler.h"

/**
* Parameters for creating a world state data writer
*/
struct FSLWorldStateWriterParams
{
	// Min linear movement (squared for efficiency) in order to log an entity
	float LinearDistanceSquared;

	// Min angular movement in order to log an entity
	float AngularDistance;

	// Location where to save the data (filename/database name etc.)
	FString Location;

	// Episode unique id
	FString EpisodeId;

	// Server ip (optional)
	FString ServerIp;

	// Server Port (optional)
	uint16 ServerPort;

	// Overwrite exiting data
	bool bOverwrite;

	// Constructor
	FSLWorldStateWriterParams(
		float InLinearDistance,
		float InAngularDistance,
		const FString& InLocation,
		const FString& InEpisodeId,
		const FString& InServerIp = "",
		uint16 InServerPort = 0,
		bool bInOverwrite = false) :
		LinearDistanceSquared(InLinearDistance*InLinearDistance),
		AngularDistance(InAngularDistance),
		Location(InLocation),
		EpisodeId(InEpisodeId),
		ServerIp(InServerIp),
		ServerPort(InServerPort),
		bOverwrite(bInOverwrite)
	{};
};


/**
 * Base class for world state data writer
 */
class ISLWorldStateWriter
{
public:
	// Virtual dtor
	virtual ~ISLWorldStateWriter(){};
	
	// Init the writer
	virtual void Init(const FSLWorldStateWriterParams& InParams) = 0;
	
	// Finish
	virtual void Finish() = 0;

	// Write data (it also checks and removes invalid items from the array - e.g. destroyed actors)
	//virtual void Write(TArray<TSLEntityPreviousPose<AActor>>& NonSkeletalActorPool,
	//	TArray<TSLEntityPreviousPose<ASLSkeletalMeshActor>>& SkeletalActorPool,
	//	TArray<TSLEntityPreviousPose<USceneComponent>>& NonSkeletalComponentPool,
	//	float Timestamp) = 0;

	virtual void Write(float Timestamp,
		TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
		TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
		TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
		FSLGazeData& GazeData,
		bool bCheckAndRemoveInvalidEntities = true) = 0;

	// True if the writer is valid
	bool IsInit() const { return bIsInit; }

protected:
	// Flag to show if it is valid
	bool bIsInit;
	
	// Location step size (log items that moved at least this distance since the last log)
	float LinDistSqMin;;

	// Rotation step size (log items that rotated at least this value since the last log)
	float AngDistMin;
	
	// Previous gaze data
	FSLGazeData PreviousGazeData;
};
