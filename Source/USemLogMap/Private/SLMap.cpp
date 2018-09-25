// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMap.h"
#include "Tags.h"

TSharedPtr<FSLMap> FSLMap::StaticInstance;

// Constructor
FSLMap::FSLMap() : bIsInit(false) 
{
}

// Destructor
FSLMap::~FSLMap() 
{
}

// Get singleton
FSLMap* FSLMap::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FSLMap());
	}
	return StaticInstance.Get();
}

// Delete instance
void FSLMap::DeleteInstance()
{
	StaticInstance.Reset();
}

// Init data
void FSLMap::LoadData(UWorld* World)
{
	// Clear any previous data
	IdSemIdMap.Empty();
	IdClassMap.Empty();
	
	IdSemIdMap = FTags::GetObjectsIdToKeyValue(World, "SemLog", "Id");
	IdClassMap = FTags::GetObjectsIdToKeyValue(World, "SemLog", "Class");

	// Mark as initialized
	bIsInit = true;
}

// Get semantic id, from unique id
FString FSLMap::GetSemanticId(uint32 UniqueId) const
{
	if (const FString* SemId = IdSemIdMap.Find(UniqueId))
	{
		return *SemId;
	}
	else
	{
		return FString();
	}
}

// Get semantic class, from unique id
FString FSLMap::GetSemanticClass(uint32 UniqueId) const
{
	if (const FString* Class = IdClassMap.Find(UniqueId))
	{
		return *Class;
	}
	else
	{
		return FString();
	}
}

