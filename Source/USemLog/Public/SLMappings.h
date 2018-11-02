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

	// Remove item from object
	bool RemoveItem(UObject* Object);

	// Remove item from unique id
	bool RemoveItem(uint32 UniqueId);

	// Try to add the given object as a semantic item (return false if the item is not properly annotated)
	bool AddItem(UObject* Object);

	// Get semantic item structure, from object
	FSLItem GetSemanticItem(UObject* Object) const;

	// Get semantic item structure, from unique id
	FSLItem GetSemanticItem(uint32 UniqueId) const;

	// Get semantic id from object
	FString GetSemanticId(UObject* Object) const;

	// Get semantic id from unique id
	FString GetSemanticId(uint32 UniqueId) const;

	// Get semantic class from object
	FString GetSemanticClass(UObject* Object) const;

	// Get semantic class from unique id
	FString GetSemanticClass(uint32 UniqueId) const;

	// Check is semantically item exists and is valid from object
	bool HasValidItem(UObject* Object) const;

	// Check is semantically item exists and is valid from unique id
	bool HasValidItem(uint32 UniqueId) const;

	// Check if object has a valid ancestor 
	bool HasValidAncestor(UObject* Object, UObject* OutAncestor = nullptr) const;

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
