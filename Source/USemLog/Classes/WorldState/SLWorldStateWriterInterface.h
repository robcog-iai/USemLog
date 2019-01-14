// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "SLStructs.h"
#include "SLSkeletalMeshActor.h"
#include "SLWorldStateWriterInterface.generated.h"


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

	// Constructor
	FSLWorldStateWriterParams(
		float InLinearDistance,
		float InAngularDistance,
		const FString& InLocation,
		const FString& InEpisodeId,
		const FString& InServerIp = "",
		uint16 InServerPort = 0) :
		LinearDistanceSquared(InLinearDistance*InLinearDistance),
		AngularDistance(InAngularDistance),
		Location(InLocation),
		EpisodeId(InEpisodeId),
		ServerIp(InServerIp),
		ServerPort(InServerPort)
	{};
};


/**
* Dummy class needed to support Cast<ISLWorldStateWriter>(Object). 
*/
UINTERFACE(Blueprintable)
class USLWorldStateWriterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Base class for world state data writer
 */
class ISLWorldStateWriterInterface
{
	GENERATED_BODY()

public:
	// Init the writer
	virtual void Init(const FSLWorldStateWriterParams& InParams) = 0;
	
	// Write data (it also removes invalid items from the array - e.g. destroyed actors)
	virtual void Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
		TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
		TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
		float Timestamp) = 0;

	// True if the writer is valid
	bool IsInit() const { return bIsInit; }

protected:
	// Flag to show if it is valid
	bool bIsInit;

	// Location step size (log items that moved at least this distance since the last log)
	float MinLinearDistanceSquared;;

	// Rotation step size (log items that rotated at least this value since the last log)
	float MinAngularDistance;
};
