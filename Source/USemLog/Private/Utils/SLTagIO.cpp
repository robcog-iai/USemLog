// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Utils/SLTagIO.h"
#include "EngineUtils.h"

// Get all tag key value pairs from world
TMap<AActor*, TMap<FString, FString>> FSLTagIO::GetWorldKVPairs(UWorld* World, const FString& TagType)
{
	TMap<AActor*, TMap<FString, FString>> ActorToKVPairs;
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		const TMap<FString, FString> KVPairs = FSLTagIO::GetKVPairs(*ActorItr, TagType);
		if (KVPairs.Num() > 0)
		{
			ActorToKVPairs.Emplace(*ActorItr, KVPairs);
		}
	}
	return ActorToKVPairs;
}

// Get tag key value pairs from actor
TMap<FString, FString> FSLTagIO::GetKVPairs(AActor* Actor, const FString& TagType)
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

// Add key value pair to the tag value
bool FSLTagIO::AddKVPair(FName& Tag, const FString& TagKey, const FString& TagValue, bool bOverwrite)
{
	// Check if the value is already present
	const FString CurrVal = FSLTagIO::GetValue(Tag, TagKey);
	if (CurrVal.IsEmpty())
	{
		// Key does not exist, add new one at the end
		Tag = FName(*AppendKV(Tag, TagKey, TagValue));
		return true;
	}
	else if (bOverwrite)
	{
		// Key exist, replace
		const FString Old = TagKey + "," + CurrVal;
		const FString New = TagKey + "," + TagValue;
		Tag = FName(*Tag.ToString().Replace(*Old, *New));
		Tag = FName(*Tag.ToString().Replace(*CurrVal, *TagValue));
		return true;
	}
	// Cannot overwrite value, return false
	return false;
}

// Add key value pair to actor
bool FSLTagIO::AddKVPair(AActor* Actor, const FString& TagType, const FString& TagKey, const FString& TagValue, bool bOverwrite)
{
	// Check if type exists and return index of its location in the array
	const int32 TagIndex = FSLTagIO::IndexOfType(Actor->Tags, TagType);
	if (TagIndex != INDEX_NONE)
	{
		if (FSLTagIO::AddKVPair(Actor->Tags[TagIndex], TagKey, TagValue, bOverwrite))
		{
			Actor->Modify();
			return true;
		}
		else
		{
			return false;
		}
	}
	else // Type was not found, create a new one
	{
		Actor->Modify();
		Actor->Tags.Add(FName(*FSLTagIO::TKVString(TagType, TagKey, TagValue)));
		return true;
	}
	return false;
}


// Get tag key value from tag
FString FSLTagIO::GetValue(const FName& InTag, const FString& TagKey)
{
	// Copy of the current tag as FString
	FString CurrTag = InTag.ToString();

	// Check the position of the key string in the tag
	int32 KeyPos = CurrTag.Find(";" + TagKey + ",");
	if (KeyPos != INDEX_NONE)
	{
		// Remove from tag with the cut length of: 
		// pos of the key + length of the semicolon char + length of the key + length of the comma char 
		CurrTag.RemoveAt(0, KeyPos + 1 + TagKey.Len() + 1);
		// Set the tag value as the left side of the string before the semicolon
		return CurrTag.Left(CurrTag.Find(";"));
	}
	// Return empty string if key was not found
	return FString();
}

// Return the index where the tag type was found in the array
int32 FSLTagIO::IndexOfType(const TArray<FName>& InTags, const FString& TagType)
{
	// Iterate all the tags, check for keyword TagType
	for (int32 i = 0; i < InTags.Num(); ++i)
	{
		// Check if tag is of given type
		if (InTags[i].ToString().StartsWith(TagType + ";"))
		{
			return i;
		}
	}
	// return INDEX_NONE if type was not found 
	return INDEX_NONE;
}

// Return a pair FName value
FString FSLTagIO::TKVString(const FString& TagType, const FString& TagKey, const FString& TagValue)
{
	return FString::Printf(TEXT("%s;%s,%s;"), *TagType, *TagKey, *TagValue);
}

// Return the Key,Value; as FName
FString FSLTagIO::KVString(const FString& TagKey, const FString& TagValue)
{
	return FString::Printf(TEXT("%s,%s;"), *TagKey, *TagValue);
}

// Return the tag as a string with the appended Key,Value; 
FString FSLTagIO::AppendKV(const FName& InTag, const FString& TagKey, const FString& TagValue)
{
	return FString::Printf(TEXT("%s%s,%s;"), *InTag.ToString(), *TagKey, *TagValue);
}
