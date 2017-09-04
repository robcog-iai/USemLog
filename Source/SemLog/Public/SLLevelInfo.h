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
USTRUCT()
struct FSLLevelProperties
{
	GENERATED_USTRUCT_BODY()

	// Metadata properties of the level
	UPROPERTY(EditAnywhere, Category = SL)
	TArray<FOwlTriple> LevelProperties;
};

/**
 * 
 */
UCLASS()
class SEMLOG_API ASLLevelInfo : public AInfo
{
	GENERATED_BODY()

public:
	// Selected episode index
	UPROPERTY(EditAnywhere, Category = SL)
	FString LevelKey;

	// Map of level description to array of properties
	UPROPERTY(EditAnywhere, Category = SL)
	TMap<FString, FSLLevelProperties> LevelKeyToProperties;
};
