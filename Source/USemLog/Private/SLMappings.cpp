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
	IdSemLogIdMap.Empty();
	IdSemLogIdMap = FTags::GetObjectsIdToKeyValue(World, "SemLog", "Id");
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	for (const auto& Pair : IdSemLogIdMap)
	{
		UE_LOG(LogTemp, Error, TEXT("\t\t[%s][%d] Pair: %i - %s"),
			TEXT(__FUNCTION__), __LINE__, Pair.Key, *Pair.Value);
	}
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
