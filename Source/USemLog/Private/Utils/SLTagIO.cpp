// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Utils/SLTagIO.h"
#include "EngineUtils.h"


// Get all tag key value pairs from world
TMap<AActor*, TMap<FString, FString>> FSLTagIO::GetAllKVPairs(UWorld* World, const FString& TagType)
{
	TMap<AActor*, TMap<FString, FString>> ActorToKVPairs;
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		const TMap<FString, FString> KVPairs = FSLTagIO::GetActorKVPairs(*ActorItr, TagType);
		if (KVPairs.Num() > 0)
		{
			ActorToKVPairs.Emplace(*ActorItr, KVPairs);
		}
	}
	return ActorToKVPairs;
}

// Get tag key value pairs from actor
TMap<FString, FString> FSLTagIO::GetActorKVPairs(AActor* Actor, const FString& TagType)
{
	TMap<FString, FString> KVPairs;
	for (const auto& TagItr : Actor->Tags)
	{
		// Copy of the current tag as FString
		FString CurrTag = TagItr.ToString();

		// Check if tag is related to the TagType
		if (CurrTag.RemoveFromStart(TagType))
		{
			// Split on semicolon
			FString CurrPair;
			while (CurrTag.Split(TEXT(";"), &CurrPair, &CurrTag))
			{
				// Split on comma
				FString CurrKey, CurrValue;
				if (CurrPair.Split(TEXT(","), &CurrKey, &CurrValue))
				{
					if (!CurrKey.IsEmpty() && !CurrValue.IsEmpty())
					{
						KVPairs.Emplace(CurrKey, CurrValue);
					}
				}
			}
		}
	}
	return KVPairs;
}
