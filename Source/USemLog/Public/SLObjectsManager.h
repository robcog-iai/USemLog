// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLStructs.h"
#include "SLSkeletalDataComponent.h"

/**
 * Singleton storing Map between the unreal objects and the semantic data
 */
class USEMLOG_API FSLObjectsManager
{
private:
	// Constructor
	FSLObjectsManager();

public:
	// Destructor
	~FSLObjectsManager();

	// Get singleton
	static FSLObjectsManager* GetInstance();

	// Delete instance
	static void DeleteInstance();

	// Init data / load Map
	void Init(UWorld* World);

	// Check if data is loaded
	bool IsInit() const { return bIsInit; }

	// Clear data
	void Clear();

	// Enable replication on the items
	void SetReplicates(bool bReplicate);
	
	// Remove object from object
	bool RemoveObject(UObject* Object);

	// Try to add the given object as a semantic object (return false if the object is not properly annotated)
	bool AddObject(UObject* Object);

	// Get semantic object structure, from object
	FSLObject GetObject(UObject* Object) const;
	
	// Get semantic id from object
	FString GetId(UObject* Object) const;

	// Get semantic class from object
	FString GetClass(UObject* Object) const;

	// Check is semantically object exists and is valid from object
	bool HasValidObject(UObject* Object) const;

	// Check if object has a valid ancestor 
	bool HasValidAncestor(UObject* Object, UObject* OutAncestor = nullptr) const;

	// Get the map of objects to the semantic items
	TMap<UObject*, FSLObject>& GetObjectsSemanticData() { return ObjectsSemanticData; }

	// Get the array of semantically annotated objects
	int32 GetSematicObjects(TArray<UObject*>& OutArray) { return ObjectsSemanticData.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GetSemanticDataArray(TArray<FSLObject>& OutArray) { ObjectsSemanticData.GenerateValueArray(OutArray); }

private:
	// Instance of the singleton
	static TSharedPtr<FSLObjectsManager> StaticInstance;

	// Flag showing the data has been init
	bool bIsInit;

	// Map of UObject pointer to object structure
	TMap<UObject*, FSLObject> ObjectsSemanticData;

	// Map of UObject to skeletal data component
	TMap<UObject*, USLSkeletalDataComponent*> ObjectsSemanticSkeletalData;
};
