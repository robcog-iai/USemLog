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
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager = nullptr);

private:
	UPROPERTY(EditAnywhere, Category = "Description")
	FString Description;
};

/**
 *
 */
UCLASS()
class USLVizQBatch : public USLVizQBase
{
	GENERATED_BODY()

public:
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager = nullptr) override;

private:
	UPROPERTY(EditAnywhere, Category = "VizQ")
	TArray<USLVizQBase*> Children;
};


/**
 *
 */
UCLASS()
class USLVizQEpisode : public USLVizQBase
{
	GENERATED_BODY()

public:
	virtual void Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager = nullptr) override;

private:
	UPROPERTY(EditAnywhere, Category = "VizQ")
	FString EpisodeId;
};
