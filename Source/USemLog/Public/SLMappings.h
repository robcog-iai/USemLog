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

	// Enable replication on the items
	void SetReplicates(bool bReplicate);

	// Clear data
	void Clear();
	
	// Remove item from object
	bool RemoveItem(UObject* Object);

	// Try to add the given object as a semantic item (return false if the item is not properly annotated)
	bool AddItem(UObject* Object);

	// Get semantic item structure, from object
	FSLItem GetItem(UObject* Object) const;
	
	// Get semantic id from object
	FString GetId(UObject* Object) const;

	// Get semantic class from object
	FString GetClass(UObject* Object) const;

	// Check is semantically item exists and is valid from object
	bool HasValidItem(UObject* Object) const;

	// Check if object has a valid ancestor 
	bool HasValidAncestor(UObject* Object, UObject* OutAncestor = nullptr) const;

	// Check if data is loaded
	bool IsInit() const { return bIsInit; }

	// Get the map of objects to the semantic items
	TMap<UObject*, FSLItem>& GetItemMap() { return ObjItemMap; }

	// Get the array of semantically annotated objects
	int32 GetObjects(TArray<UObject*>& OutArray) { return ObjItemMap.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GenerateItemsArray(TArray<FSLItem>& OutArray) { ObjItemMap.GenerateValueArray(OutArray); }

private:
	// Instance of the singleton
	static TSharedPtr<FSLMappings> StaticInstance;

	// Flag showing the data has been init
	bool bIsInit;

	// Map of UObject pointer to item structure
	TMap<UObject*, FSLItem> ObjItemMap;
};
