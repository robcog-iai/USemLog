// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLTagIO.generated.h"

/*
* TagType;Key1,Value1;Key2,Value2;Key3,Value3;
*/
USTRUCT()
struct USEMLOG_API FSLTagIO
{
	GENERATED_BODY();

	// Get all tag key value pairs from world
	static TMap<AActor*, TMap<FString, FString>> GetAllKVPairs(UWorld* World, const FString& TagType);

	// Get tag key value pairs from actor
	static TMap<FString, FString> GetActorKVPairs(AActor* Actor, const FString& TagType);
};
