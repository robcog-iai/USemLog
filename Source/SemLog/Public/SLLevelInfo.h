// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLOwl.h"
#include "SLLevelInfo.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API ASLLevelInfo : public AInfo
{
	GENERATED_BODY()

public:
	// Metadata properties of the level
	UPROPERTY(EditAnywhere, Category = SL)
	TArray<FOwlTriple> LevelProperties;
};
