// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMappings.h"
#include "Tags.h"

TSharedPtr<FSLMappings> FSLMappings::StaticInstance;

// Constructor
FSLMappings::FSLMappings()
{
}

// Destructor
FSLMappings::~FSLMappings()
{
	IdSemLogIdMap.Empty();
}

// Get singleton
FSLMappings* FSLMappings::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FSLContentSingleton());
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
	IdSemLogIdMap.Empty();
	IdSemLogIdMap = FTags::GetObjectsIdToKeyValue(World, "SemLog", "Id");
}

// Get semantic id, from unique id
FString FSLMappings::GetSemLogId(uint32 UniqueId) const
{
	if (const FString* SemLogId = IdSemLogIdMap.Find(UniqueId))
	{
		return *SemLogId;
	}
	else
	{
		return FString();
	}
}
