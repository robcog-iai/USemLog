// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Utils/SLTagIO.h"
#include "EngineUtils.h"

/* Read */
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
		FString CurrTagCopy = TagItr.ToString();

		// Check if tag is related to the TagType
		if (CurrTagCopy.StartsWith(TagType + ";") && CurrTagCopy.RemoveFromStart(TagType))
		{
			// Split on semicolon
			FString CurrPair;
			while (CurrTagCopy.Split(TEXT(";"), &CurrPair, &CurrTagCopy))
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

// Get tag key value from actor
FString FSLTagIO::GetValue(AActor* Actor, const FString& TagType, const FString& TagKey)
{
	// Check if type exists and return index of its location in the array
	int32 TagIndex = IndexOfType(Actor->Tags, TagType);
	if (TagIndex != INDEX_NONE)
	{
		return GetValue(Actor->Tags[TagIndex], TagKey);
	}
	// Tag type not found
	return FString();
}

// Check if key exists
bool FSLTagIO::HasKey(AActor* Actor, const FString& TagType, const FString& TagKey)
{
	// Check if type exists and return index of its location in the array
	int32 TagIndex = IndexOfType(Actor->Tags, TagType);
	if (TagIndex != INDEX_NONE)
	{
		return Actor->Tags[TagIndex].ToString().Find(";" + TagKey + ",") != INDEX_NONE;
	}
	// Type was not found, return false
	return false;
}

// Check if type exists, optionally return the position in the array
bool FSLTagIO::HasType(AActor* Actor, const FString& TagType, int32* OutPos)
{
	if (OutPos)
	{
		*OutPos = IndexOfType(Actor->Tags, TagType);
		return *OutPos != INDEX_NONE;
	}
	else
	{
		return IndexOfType(Actor->Tags, TagType) != INDEX_NONE;
	}
}


/* Create / Update */
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


/* Delete */
// Remove all tag key value pairs from world
bool FSLTagIO::RemoveWorldKVPairs(UWorld* World, const FString& TagType, const FString& TagKey)
{
	bool bRemovedAny = false;
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		bRemovedAny = bRemovedAny || FSLTagIO::RemoveKVPair(*ActorItr, TagType, TagKey);
	}
	return bRemovedAny;
}

// Remove the pair with the given type and key (return true if the key existed)
bool FSLTagIO::RemoveKVPair(AActor* Actor, const FString& TagType, const FString& TagKey)
{
	// Check if type exists and return index of its location in the array
	int32 TagIndex = IndexOfType(Actor->Tags, TagType);
	if (TagIndex != INDEX_NONE)
	{
		// Copy of the current tag as FString
		FString TagStr = Actor->Tags[TagIndex].ToString();
		const FString ToRemove = TagKey + TEXT(",") + GetValue(Actor->Tags[TagIndex], TagKey) + TEXT(";");
		int32 FindPos = TagStr.Find(ToRemove, ESearchCase::CaseSensitive);
		if (FindPos != INDEX_NONE)
		{
			Actor->Modify();
			TagStr.RemoveAt(FindPos, ToRemove.Len());
			Actor->Tags[TagIndex] = FName(*TagStr);
			return true;
		}
		// "TagKey,TagValue;" combo could not be found
		return false;
	}
	// Tag type not found, nothing to remove
	return false;
}





/* Utils */
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

// Get tag key value from tag
FString FSLTagIO::GetValue(const FName& InTag, const FString& TagKey)
{
	// Copy of the current tag as FString
	FString CurrTagCopy = InTag.ToString();

	// Check the position of the key string in the tag
	int32 KeyPos = CurrTagCopy.Find(";" + TagKey + ",");
	if (KeyPos != INDEX_NONE)
	{
		// Remove from tag with the cut length of: 
		// pos of the key + length of the semicolon char + length of the key + length of the comma char 
		CurrTagCopy.RemoveAt(0, KeyPos + 1 + TagKey.Len() + 1);
		// Set the tag value as the left side of the string before the semicolon
		return CurrTagCopy.Left(CurrTagCopy.Find(";"));
	}
	// Return empty string if key was not found
	return FString();
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
