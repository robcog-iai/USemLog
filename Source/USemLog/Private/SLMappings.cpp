// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMappings.h"
#include "Tags.h"

TSharedPtr<FSLMappings> FSLMappings::StaticInstance;

// Constructor
FSLMappings::FSLMappings() : bIsInit(false) 
{
}

// Destructor
FSLMappings::~FSLMappings() 
{
}

// Get singleton
FSLMappings* FSLMappings::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FSLMappings());
	}
	return StaticInstance.Get();
}

// Delete instance
void FSLMappings::DeleteInstance()
{
	StaticInstance.Reset();
}

// Init data
void FSLMappings::LoadData(UWorld* World)
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
FString FSLMappings::GetSemanticId(uint32 UniqueId) const
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
FString FSLMappings::GetSemanticClass(uint32 UniqueId) const
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

