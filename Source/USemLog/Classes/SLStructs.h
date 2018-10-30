#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
* Structure holding the semantic data of one item
*/
struct FSLItem
{
	// Unique UObject id of item
	uint32 Id;

	// Semantic id of item
	FString SemId;

	// Semantic class of item
	FString Class;

	// Default constructor
	FSLItem() {};

	// Init constructor
	FSLItem(uint32 InId, const FString& InSemId, const FString& InSemClass) : 
		Id(InId), SemId(InSemId), Class(InSemClass) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsValid() const { return Id > 0 && !SemId.IsEmpty() && !Class.IsEmpty(); }

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("Id:%ld SemId:%s Class:%s"), Id, *SemId, *Class);
	}
};

/**
* Structure holding the semantic data of two items
*/
struct FSLItemPair
{
	// First item
	FSLItem Item1;

	// Second item
	FSLItem Item2;

	// Default constructor
	FSLItemPair() {};

	// Init constructor
	FSLItemPair(const FSLItem& InItem1, const FSLItem& InItem2) : Item1(InItem1), Item2(InItem2) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsValid() const { return Item1.IsValid() && Item2.IsValid(); }

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("%s %s"), *Item1.ToString(), *Item2.ToString());
	}
};

