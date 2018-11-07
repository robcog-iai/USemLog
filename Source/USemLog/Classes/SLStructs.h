#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
* Structure holding the semantic data of one item
*/
struct FSLItem
{
	// UObject of item
	UObject* Obj;

	// Semantic id of item
	FString Id;

	// Semantic class of item
	FString Class;

	// Default constructor
	FSLItem() {};

	// Init constructor
	FSLItem(UObject* InObj, const FString& InId, const FString& InClass) :
		Obj(InObj), Id(InId), Class(InClass) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsValid() const { return Obj != nullptr && !Id.IsEmpty() && !Class.IsEmpty(); }

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("UniqueID:%ld Id:%s Class:%s"), Obj->GetUniqueID(), *Id, *Class);
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


/**
* Raw data structure for the logged entities
*/
template <typename T>
struct TSLItemState
{
	// The semantically annotated item
	FSLItem Item;

	// Its previous location
	FVector PrevLoc;

	// Its previous rotation
	FQuat PrevQuat;

	// Pointer to the actor/component/skeletal actor
	TWeakObjectPtr<T> Entity;

	// Default constructor
	TSLItemState() {};

	// Init constructor
	TSLItemState(const FSLItem& InItem,
		TWeakObjectPtr<T> InEntity,
		FVector InPrevLoc = FVector(BIG_NUMBER),
		FQuat InPrevQuat = FQuat::Identity) :
		Item(InItem),
		Entity(InEntity),
		PrevLoc(InPrevLoc),
		PrevQuat(InPrevQuat)
	{};
};

