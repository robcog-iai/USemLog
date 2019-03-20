// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEntitiesManager.h"
#include "Tags.h"

TSharedPtr<FSLEntitiesManager> FSLEntitiesManager::StaticInstance;

// Constructor
FSLEntitiesManager::FSLEntitiesManager() : bIsInit(false) 
{
}

// Destructor
FSLEntitiesManager::~FSLEntitiesManager() 
{
}

// Get singleton
FSLEntitiesManager* FSLEntitiesManager::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FSLEntitiesManager());
	}
	return StaticInstance.Get();
}

// Delete instance
void FSLEntitiesManager::DeleteInstance()
{
	StaticInstance.Reset();
}

// Init data
void FSLEntitiesManager::Init(UWorld* World)
{
	if (!bIsInit)
	{
		// Iterate all actors
		for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			// Add to map if key is found in the actor
			FString ActId = FTags::GetValue(*ActorItr, "SemLog", "Id");
			FString ActClass = FTags::GetValue(*ActorItr, "SemLog", "Class");
			if (!ActId.IsEmpty() && !ActClass.IsEmpty())
			{
				ObjectsSemanticData.Emplace(*ActorItr, FSLEntity(*ActorItr, ActId, ActClass));
			}

			// Iterate components of the actor
			for (const auto& CompItr : ActorItr->GetComponents())
			{
				// Add to map if key is found in the actor
				FString CompId = FTags::GetValue(CompItr, "SemLog", "Id");
				FString CompClass = FTags::GetValue(CompItr, "SemLog", "Class");
				if (!CompId.IsEmpty() && !CompClass.IsEmpty())
				{
					ObjectsSemanticData.Emplace(CompItr, FSLEntity(CompItr, CompId, CompClass));
				}

				// Check if the component is a skeletal data container
				if (USLSkeletalDataComponent* AsSkelData = Cast<USLSkeletalDataComponent>(CompItr))
				{
					if (AsSkelData->Init())
					{
						ObjectsSemanticSkeletalData.Add(AsSkelData->OwnerSemanticData.Obj, AsSkelData);
					}
				}
			}
		}

		// Mark as initialized
		bIsInit = true;
	}
}


// Clear data
void FSLEntitiesManager::Clear()
{
	// Clear any previous data
	ObjectsSemanticData.Empty();

	// Mark as uninitialized
	bIsInit = false;
}

// Enable replication on the items
void FSLEntitiesManager::SetReplicates(bool bReplicate)
{
	for (auto& Pair : ObjectsSemanticData)
	{
		//if (AActor* ObjAsActor = Cast<AActor>(Pair.Key))
		//{
		//	ObjAsActor->SetReplicates(bReplicate);
		//	ObjAsActor->SetReplicateMovement(bReplicate);
		//}
		//else if (UActorComponent* ObjAsActorComponent = Cast<UActorComponent>(Pair.Key))
		//{
		//	ObjAsActorComponent->SetIsReplicated(bReplicate);
		//	ObjAsActorComponent->GetOwner()->SetReplicates(bReplicate);
		//	ObjAsActorComponent->GetOwner()->SetReplicateMovement(bReplicate);
		//}

		// Only replicate items with visuals
		if (AActor* ObjAsSMA = Cast<AStaticMeshActor>(Pair.Key))
		{
			if (ObjAsSMA->IsRootComponentMovable())
			{
				ObjAsSMA->SetReplicates(bReplicate);
				ObjAsSMA->SetReplicateMovement(bReplicate);
			}
		}
		else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(Pair.Key))
		{
			if (ObjAsSMC->Mobility == EComponentMobility::Movable)
			{
				ObjAsSMC->SetIsReplicated(bReplicate);
				ObjAsSMC->GetOwner()->SetReplicates(bReplicate);
				ObjAsSMC->GetOwner()->SetReplicateMovement(bReplicate);
			}
		}
		// TODO see skeletal mesh comps and actors
	}
}


// Remove object from object
bool FSLEntitiesManager::RemoveEntity(UObject* Object)
{
	//return FSLMappings::RemoveItem(Object->GetUniqueID());
	int32 NrOfRemovedItems = ObjectsSemanticData.Remove(Object);
	if (NrOfRemovedItems > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Try to add the given object as a semantic object (return false if the object is not properly annotated)
bool FSLEntitiesManager::AddObject(UObject* Object)
{
	// Add to map if key is found in the actor
	FString Id = FTags::GetValue(Object, "SemLog", "Id");
	FString Class = FTags::GetValue(Object, "SemLog", "Class");
	if (!Id.IsEmpty() && !Class.IsEmpty())
	{
		ObjectsSemanticData.Emplace(Object, FSLEntity(Object, Id, Class));
		return true;
	}
	else
	{
		return false;
	}
}

// Get semantic object structure, from object
FSLEntity FSLEntitiesManager::GetEntity(UObject* Object) const
{
	//return FSLMappings::GetSemanticObject(Object->GetUniqueID());
	if (const FSLEntity* Item = ObjectsSemanticData.Find(Object))
	{
		return *Item;
	}
	else
	{
		return FSLEntity();
	}
}

// Get semantic id from object
FString FSLEntitiesManager::GetId(UObject* Object) const
{
	//return FSLMappings::GetSemanticId(Object->GetUniqueID());
	if (const FSLEntity* Item = ObjectsSemanticData.Find(Object))
	{
		return *Item->Id;
	}
	else
	{
		return FString();
	}
}

// Get semantic class from object
FString FSLEntitiesManager::GetClass(UObject* Object) const
{
	//return FSLMappings::GetSemanticClass(Object->GetUniqueID());
	if (const FSLEntity* Item = ObjectsSemanticData.Find(Object))
	{
		return *Item->Class;
	}
	else
	{
		return FString();
	}
}

// Check is semantically object exists and is valid from object
bool FSLEntitiesManager::IsObjectEntitySet(UObject* Object) const
{
	//return FSLMappings::HasValidItem(Object->GetUniqueID());
	if (const FSLEntity* Item = ObjectsSemanticData.Find(Object))
	{
		return Item->IsSet();
	}
	else
	{
		return false;
	}
}

// Check if object has a valid ancestor 
bool FSLEntitiesManager::GetValidAncestor(UObject* Object, UObject* OutAncestor) const
{
	UObject* Child = Object;
	while(UObject* Outer = Child->GetOuter())
	{
		if (FSLEntitiesManager::IsObjectEntitySet(Outer))
		{
			OutAncestor = Outer;
			return true;
		}
		else
		{
			// Move up on the tree
			Child = Outer;
		}
	}
	return false;
}