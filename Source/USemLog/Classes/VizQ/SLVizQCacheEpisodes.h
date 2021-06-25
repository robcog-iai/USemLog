// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "VizQ/SLVizQBase.h"
#include "SLVizQCacheEpisodes.generated.h"

// Forward declaration
class ASLKnowrobManager;

/**
 * 
 */
UCLASS()
class USLVizQCacheEpisodes : public USLVizQBase
{
	GENERATED_BODY()

protected:
	// Virtual implementation of the execute function
	virtual void ExecuteImpl(ASLKnowrobManager* KRManager) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Cache Episodes")
	FString Task;

	UPROPERTY(EditAnywhere, Category = "Cache Episodes")
	TArray<FString> Episodes;
};
