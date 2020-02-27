// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#if SL_WITH_DATA_VIS
#include "VizMarker.h"
#endif //SL_WITH_DATA_VIS
#include "SLDataVisQueries.generated.h"

/*
* Marker primitive types
*/
UENUM()
enum class EVizMarkerTypeVQ : uint8
{
	Box				UMETA(DisplayName = "Box"),
	Sphere			UMETA(DisplayName = "Sphere"),
	Cylinder		UMETA(DisplayName = "Cylinder"),
	Arrow			UMETA(DisplayName = "Arrow"),
	Axis			UMETA(DisplayName = "Axis"),
};

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
 * Action types
 */
UENUM()
enum class ESLVisMarkerType : uint8
{
	None				UMETA(DisplayName = NONE),
	PrimitiveMesh		UMETA(DisplayName = PrimitiveMesh),
	StaticMesh			UMETA(DisplayName = StaticMesh),
	SkeletalMesh		UMETA(DisplayName = SkeletalMesh),
	Highlight			UMETA(DisplayName = Highlight),

	Replay				UMETA(DisplayName = Replay),
	Trajectory			UMETA(DisplayName = Trajectory),
	ClearAll			UMETA(DisplayName = ClearAll),
};

/**
 * Query structure
 */
USTRUCT()
struct FSLVisMarker
{
	GENERATED_BODY()

	// Default ctor
	FSLVisMarker() {};

	// Action type
	UPROPERTY(EditAnywhere)
	ESLVisMarkerType MarkerType = ESLVisMarkerType::None;

	// Color
	UPROPERTY(EditAnywhere)
	FLinearColor Color = FLinearColor::Green;

	// Marker lit type
	UPROPERTY(EditAnywhere)
	bool bUnlit = false;

	// Material to draw index
	UPROPERTY(EditAnywhere)
	int32 MaterialIndex = INDEX_NONE;

	// Use original color
	UPROPERTY(EditAnywhere)
	bool bUseOriginalMaterials = false;

	// Scale
	UPROPERTY(EditAnywhere)
	FVector Scale = FVector(0.1);

	UPROPERTY(EditAnywhere)
	EVizMarkerTypeVQ PrimitiveMarkerType = EVizMarkerTypeVQ::Box;
};

/**
 * Query structure
 */
USTRUCT()
struct FSLVisQuery
{
	GENERATED_BODY()

	// Task id (mongo database name)
	UPROPERTY(EditAnywhere, Category="Query")
	FString TaskId;

	// Episode id (mongo collection name)
	UPROPERTY(EditAnywhere, Category = "Query")
	FString EpisodeId;

	// Query type
	UPROPERTY(EditAnywhere, Category = "Query")
	ESLVisQueryType QueryType;

	// Id of the entity one is searching for
	UPROPERTY(EditAnywhere, Category = "Query")
	FString EntityId;

	// Used for skeletal bone searches only
	UPROPERTY(EditAnywhere, Category = "Query")
	FString BoneName;

	// Query timestamp, or start time for trajectories/timelines
	UPROPERTY(EditAnywhere, Category = "Query")
	float StartTimestamp;

	// End time for trajectories/timelines
	UPROPERTY(EditAnywhere, Category = "Query")
	float EndTimestamp = -1.f;

	// Delta time for trajectories
	UPROPERTY(EditAnywhere, Category = "Query")
	float DeltaT = -1.f;

	// Delta time for trajectories
	UPROPERTY(EditAnywhere)
	FSLVisMarker Action;

	// Skip this query
	UPROPERTY(EditAnywhere)
	bool bSkip = false;

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
