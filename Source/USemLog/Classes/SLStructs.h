#pragma once
// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
* Structure holding the semantic data of an item
*/
struct FSLItem
{
	// Unique UObject id of other
	uint32 Id;

	// Semantic id of other
	FString SemId;

	// Semantic class of other
	FString SemClass;

	// Default constructor
	FSLItem() {};

	// Init constructor
	FSLItem(uint32 InId, const FString& InSemId, const FString& InSemClass) : Id(InId), SemId(InSemId), SemClass(InSemClass) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsValid() const { return Id > 0 && !SemId.IsEmpty() && !SemClass.IsEmpty(); }
};
