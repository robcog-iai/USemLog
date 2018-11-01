// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLStructs.h"

/**
 * Singleton storing Map between the unreal objects and the semantic data
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

	// Init data / load Map
	void Init(UWorld* World);

	// Clear data
	void Clear();

	// Remove item
	bool RemoveItem(uint32 UniqueId);

	// Remove item
	bool AddItem(UObject* Object);

	// Get semantic item structure, from unique id
	FSLItem GetSemanticItem(uint32 UniqueId) const;

	// Get semantic id from unique id
	FString GetSemanticId(uint32 UniqueId) const;

	// Get semantic class from unique id
	FString GetSemanticClass(uint32 UniqueId) const;

	// Check if data is loaded
	bool IsInit() const { return bIsInit; }

private:
	// Instance of the singleton
	static TSharedPtr<FSLMappings> StaticInstance;

	// Flag showing the data has been init
	bool bIsInit;

	// Unique id to SL Item
	TMap<uint32, FSLItem> IdItemMap;
};
