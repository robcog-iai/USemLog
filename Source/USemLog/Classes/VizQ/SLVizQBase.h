// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLVizQBase.generated.h"

// Forward declaration
class ASLVizManager;
class ASLMongoQueryManager;

/**
 * 
 */
UCLASS()
class USLVizQBase : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager);

protected:
	UPROPERTY(EditAnywhere, Category = "Description")
	FString Description;

	UPROPERTY(EditAnywhere, Category = "Children")
	TArray<USLVizQBase*> Children;
};

/**
 *
 */
UCLASS()
class USLVizQGotoFrame : public USLVizQBase
{
	GENERATED_BODY()

public:
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager) override;

private:
	UPROPERTY(EditAnywhere, Category = "Id")
	FString EpisodeId;

	// For negative value it will start from first frame
	UPROPERTY(EditAnywhere, Category = "Time")
	float Timestamp = -1.f;
};

/**
 *
 */
UCLASS()
class USLVizQReplayEpisode : public USLVizQBase
{
	GENERATED_BODY()

public:
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager) override;

private:
	UPROPERTY(EditAnywhere, Category = "Id")
	FString EpisodeId;

	// For negative value it will start from first frame
	UPROPERTY(EditAnywhere, Category = "Time")
	float StartTime = -1.f;

	// For negative value it will run until the last frame
	UPROPERTY(EditAnywhere, Category = "Time")
	float EndTime = -1.f;

	// Repeat replay after finishing
	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bLoop = true;

	// How quickly to move to the next frame (if negative, it will calculate an average update rate from the episode data)
	UPROPERTY(EditAnywhere, Category = "Properties")
	float UpdateRate = -1.f;

	// How many steps to update every frame
	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 StepSize = 1;
};



/**
 *
 */
UCLASS()
class USLVizQCacheEpisodes : public USLVizQBase
{
	GENERATED_BODY()

public:
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager) override;

private:
	UPROPERTY(EditAnywhere, Category = "VizQ")
	FString TaskId;

	UPROPERTY(EditAnywhere, Category = "VizQ")
	TArray<FString> EpisodeIds;
};

