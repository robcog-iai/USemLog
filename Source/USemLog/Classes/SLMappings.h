// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
 * Singleton storing mappings between the unreal objects and the semantic data
 */
class USEMLOG_API FSLMappings
{
private:
	// Constructor
	FSLMappings();

public:
	// Destructor
	~FSLMappings();

	// Get singleton
	static FSLMappings* GetInstance();

	// Delete instance
	static void DeleteInstance();

	// Init data / load mappings
	void LoadData(UWorld* World);

	// Get semantic id, from unique id
	FString GetSemLogId(uint32 UniqueId) const;

private:
	// Instance of the singleton
	static TSharedPtr<FSLMappings> StaticInstance;

	// Unreal unique id to SemLog unique id map
	TMap<uint32, FString> IdSemLogIdMap;
};
