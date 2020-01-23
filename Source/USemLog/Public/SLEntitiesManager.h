// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLStructs.h"
#include "SLSkeletalDataComponent.h"
#include "SLVisionCamera.h"

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
	TMap<UObject*, USLSkeletalDataComponent*>& GetObjectsSkeletalSemanticData() { return ObjectsSemanticSkelData; }

	// Get the array of semantically annotated objects (returns the number of keys)
	int32 GetSemanticSkeletalObjects(TArray<UObject*>& OutArray) const { return ObjectsSemanticSkelData.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GetSemanticSkeletalDataArray(TArray<USLSkeletalDataComponent*>& OutArray) const { ObjectsSemanticSkelData.GenerateValueArray(OutArray); }

	
	// Get the map of objects to the semantic items
	TMap<ASLVisionCamera*, FSLEntity>& GetCameraViewsSemanticData() { return CameraViewSemanticData; }

	// Get the array of semantically annotated objects (returns the number of keys)
	int32 GetCameraViewsObjects(TArray<ASLVisionCamera*>& OutArray) const { return CameraViewSemanticData.GetKeys(OutArray); }

	// Get the map of objects to the semantic items
	void GetCameraViewsDataArray(TArray<FSLEntity>& OutArray) const { CameraViewSemanticData.GenerateValueArray(OutArray); }

	// Check if there are any empty of duplicate values in the camera views
	bool EmptyOrDuplicatesInTheCameraViews();

	// Get static mesh actor
	FORCEINLINE AStaticMeshActor* GetStaticMeshActor(const FString& Id) const 
	{
		if(auto* Value = IdToStaticMeshActor.Find(Id))
		{
			return *Value;
		}
		return nullptr;
	};

	// Get all the semantically annotated static mesh actors
	void GetStaticMeshActors(TArray<AStaticMeshActor*>& OutArray) const
	{
		return IdToStaticMeshActor.GenerateValueArray(OutArray);
	}

	// Get skeletal mesh actor
	FORCEINLINE ASkeletalMeshActor* GetSkeletalMeshActor(const FString& Id) const 
	{
		if(auto* Value = IdToSkeletalMeshActor.Find(Id))
			{
				return *Value;
			}
			return nullptr;
	};

	// Get all the semantically annotated skeletal mesh actors
	void GetSkeletalMeshActors(TArray<ASkeletalMeshActor*>& OutArray) const
	{
		return IdToSkeletalMeshActor.GenerateValueArray(OutArray);
	}

	// Get the semantically annotated actor
	FORCEINLINE AActor* GetActor(const FString& Id) const 
	{
		if(auto* Value = IdToActor.Find(Id))
			{
				return *Value;
			}
			return nullptr;
	};

	// Get all semantically annotated actors
	void GetActors(TArray<AActor*>& OutArray) const
	{
		IdToActor.GenerateValueArray(OutArray);
	}

	// Get all un-tagged actors
	const TArray<AActor*> GetUntaggedActors() const
	{
		return UntaggedActors;
	}

	// Get entity id (empty string if not found)
	FORCEINLINE FString GetEntityId(UObject* Object) const 
	{
		if(auto* Value = ObjectsSemanticData.Find(Object))
		{
			return Value->Id;
		}
		return FString();
	};

	// Get skeletal entity id (empty string if not found)
	FORCEINLINE FString GetSkeletalId(UObject* Object) const 
	{
		if(auto* Value = ObjectsSemanticSkelData.Find(Object))
		{
			return (*Value)->GetId();
		}
		return FString();
	};
	
private:
	// Instance of the singleton
	static TSharedPtr<FSLEntitiesManager> StaticInstance;

	// Flag showing the data has been init
	bool bIsInit;

	// TODO remove UObject and use AActor as the uppermost class type
	// Map of UObject pointer to object structure
	TMap<UObject*, FSLEntity> ObjectsSemanticData;
	//TMap<AActor*, FSLEntity> ActorSemanticData;

	// Map of UObject (Owner -- actor or component) to skeletal data component
	TMap<UObject*, USLSkeletalDataComponent*> ObjectsSemanticSkelData;

	// Map of Camera View Actors pointer to object structure
	TMap<ASLVisionCamera*, FSLEntity> CameraViewSemanticData;

	// Id to static mesh actor
	TMap<FString, AStaticMeshActor*> IdToStaticMeshActor;

	// Id to static mesh actor
	TMap<FString, ASkeletalMeshActor*> IdToSkeletalMeshActor;

	// Id to AActor
	TMap<FString, AActor*> IdToActor;

	// Un-tagged actors
	TArray<AActor*> UntaggedActors;
};
