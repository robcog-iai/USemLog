// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLStructs.h"
#include "SLSkeletalDataComponent.h"

class AStaticMeshActor;
class ASkeletalMeshActor;

/**
 * Singleton storing mappings between the unreal objects and the semantic data
 */
class USEMLOG_API FSLEntitiesManager
{
private:
	// Constructor
	FSLEntitiesManager();

public:
	// Destructor
	~FSLEntitiesManager() = default;

	// Get singleton
	static FSLEntitiesManager* GetInstance();

	// Delete instance
	static void DeleteInstance();

	// Init data / load Map
	void Init(UWorld* World);

	// Check if data is loaded
	bool IsInit() const { return bIsInit; }

	// Clear data
	void Clear();

	// Enable replication on the items
	void SetReplicates(bool bReplicate, float Priority = 1.f, float MinFreq = 2.f, float MaxFreq = 100.f);
	
	// Remove object from object
	bool RemoveEntity(UObject* Object);

	// Try to add the given object as a semantic object (return false if the object is not properly annotated)
	bool AddObject(UObject* Object);

	// Get semantic object structure, from object
	FSLEntity GetEntity(UObject* Object) const;

	// Get semantic object structure, from object
	FSLEntity* GetEntityPtr(UObject* Object);

	// Get semantic object structure, from object
	bool GetEntity(UObject* Object, FSLEntity& OutEntity) const;

	// Get semantic id from object
	FString GetId(UObject* Object) const;

	// Get semantic class from object
	FString GetClass(UObject* Object) const;

	// Check is semantically object exists and is valid from object
	bool IsObjectEntitySet(UObject* Object) const;

	// Check if object has a valid ancestor 
	bool GetValidAncestor(UObject* Object, UObject* OutAncestor = nullptr) const;

	// Get the map of objects to the semantic items
	TMap<UObject*, FSLEntity>& GetObjectsSemanticData() { return ObjectsSemanticData; }

	// Get the array of semantically annotated objects (returns the number of keys)
	int32 GetSemanticObjects(TArray<UObject*>& OutArray) const { return ObjectsSemanticData.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GetSemanticDataArray(TArray<FSLEntity>& OutArray) const { ObjectsSemanticData.GenerateValueArray(OutArray); }


	// Get the map of objects to the semantic items
	TMap<UObject*, USLSkeletalDataComponent*>& GetObjectsSkeletalSemanticData() { return ObjectsSemanticSkeletalData; }

	// Get the array of semantically annotated objects (returns the number of keys)
	int32 GetSemanticSkeltalObjects(TArray<UObject*>& OutArray) const { return ObjectsSemanticSkeletalData.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GetSemanticSkeletalDataArray(TArray<USLSkeletalDataComponent*>& OutArray) const { ObjectsSemanticSkeletalData.GenerateValueArray(OutArray); }


	// Get the map of objects to the semantic items
	TMap<UObject*, FSLEntity>& GetCameraViewsSemanticData() { return CameraViewSemanticData; }

	// Get the array of semantically annotated objects (returns the number of keys)
	int32 GetCameraViewsObjects(TArray<UObject*>& OutArray) const { return CameraViewSemanticData.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GetCameraViewsDataArray(TArray<FSLEntity>& OutArray) const { CameraViewSemanticData.GenerateValueArray(OutArray); }

	// Get static mesh actor
	FORCEINLINE AStaticMeshActor* GetStaticMeshActor(const FString& Id) const 
	{
		if(auto* Value = IdToStaticMeshActor.Find(Id))
		{
			return *Value;
		}
		return nullptr;
	};

	// Get skeletal mesh actor
	FORCEINLINE ASkeletalMeshActor* GetSkeletalMeshActor(const FString& Id) const 
	{
	if(auto* Value = IdToSkeletalMeshActor.Find(Id))
		{
			return *Value;
		}
		return nullptr;
	};

private:
	// Instance of the singleton
	static TSharedPtr<FSLEntitiesManager> StaticInstance;

	// Flag showing the data has been init
	bool bIsInit;

	// Map of UObject pointer to object structure
	TMap<UObject*, FSLEntity> ObjectsSemanticData;

	// Map of UObject (Owner -- actor or component) to skeletal data component
	TMap<UObject*, USLSkeletalDataComponent*> ObjectsSemanticSkeletalData;

	// Map of Camera View Actors pointer to object structure
	TMap<UObject*, FSLEntity> CameraViewSemanticData;

	// Id to static mesh actor
	TMap<FString, AStaticMeshActor*> IdToStaticMeshActor;

	// Id to static mesh actor
	TMap<FString, ASkeletalMeshActor*> IdToSkeletalMeshActor;
};
