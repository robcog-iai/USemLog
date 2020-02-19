// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLDataVisQueries.generated.h"

/**
 * Query types
 */
UENUM()
enum class ESLVisQueryType : uint8
{
	None				UMETA(DisplayName = NONE),
	EntityPose			UMETA(DisplayName = EntityPose),
	EntityTraj			UMETA(DisplayName = EntityTraj),
	BonePose			UMETA(DisplayName = BonePose),
	BoneTraj			UMETA(DisplayName = BoneTraj),
	SkeletalPose		UMETA(DisplayName = SkeletalPose),
	SkeletalTraj		UMETA(DisplayName = SkeletalTraj),
	GazePose			UMETA(DisplayName = GazePose),
	GazeTraj			UMETA(DisplayName = GazeTraj),
	WorldState			UMETA(DisplayName = WorldState),
	AllWorldStates		UMETA(DisplayName = AllWorldStates),
};

/**
 * Query structure
 */
USTRUCT()
struct FSLVisQuery
{
	GENERATED_BODY()

	// Task id (mongo database name)
	UPROPERTY(EditAnywhere)
	FString TaskId;

	// Episode id (mongo collection name)
	UPROPERTY(EditAnywhere)
	FString EpisodeId;

	// Query type
	UPROPERTY(EditAnywhere)
	ESLVisQueryType QueryType;

	// Id of the entity one is searching for
	UPROPERTY(EditAnywhere)
	FString EntityId;

	// Used for skeletal bone searches only
	UPROPERTY(EditAnywhere)
	FString BoneName;

	// Query timestamp, or start time for trajectories/timelines
	UPROPERTY(EditAnywhere)
	float StartTimestamp;

	// End time for trajectories/timelines
	UPROPERTY(EditAnywhere)
	float EndTimestamp;

	// End time for trajectories/timelines
	UPROPERTY(EditAnywhere)
	FString Description;
};

/**
 * 
 */
UCLASS(ClassGroup = (SL), meta = (DisplayName = "SL Data Vis Queries"))
class USLDataVisQueries : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString ServerIP = "localhost";

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint16 ServerPort = 27017;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<FSLVisQuery> Queries;
};
