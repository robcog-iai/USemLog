// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLObjectsManager.h"
#include "Tags.h"

TSharedPtr<FSLObjectsManager> FSLObjectsManager::StaticInstance;

// Constructor
FSLObjectsManager::FSLObjectsManager() : bIsInit(false) 
{
}

// Destructor
FSLObjectsManager::~FSLObjectsManager() 
{
}

// Get singleton
FSLObjectsManager* FSLObjectsManager::GetInstance()
{
	if (!StaticInstance.IsValid())
	{
		StaticInstance = MakeShareable(new FSLObjectsManager());
	}
	return StaticInstance.Get();
}

// Delete instance
void FSLObjectsManager::DeleteInstance()
{
	StaticInstance.Reset();
}

// Init data
void FSLObjectsManager::Init(UWorld* World)
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
				ObjectsSemanticData.Emplace(*ActorItr, FSLObject(*ActorItr, ActId, ActClass));
			}

			// Iterate components of the actor
			for (const auto& CompItr : ActorItr->GetComponents())
			{
				// Add to map if key is found in the actor
				FString CompId = FTags::GetValue(CompItr, "SemLog", "Id");
				FString CompClass = FTags::GetValue(CompItr, "SemLog", "Class");
				if (!CompId.IsEmpty() && !CompClass.IsEmpty())
				{
					ObjectsSemanticData.Emplace(CompItr, FSLObject(CompItr, CompId, CompClass));
				}
			}
		}

		// Iterate skeletal data components
		for (TObjectIterator<USLSkeletalDataComponent> Itr; Itr; ++Itr)
		{
			// Check if initialization is was successful
			if (Itr->Init())
			{
				ObjectsSemanticSkeletalData.Add(Itr->GetOwnerSemanticData()->Obj, *Itr);
			}
		}

		// Mark as initialized
		bIsInit = true;
	}
}


// Clear data
void FSLObjectsManager::Clear()
{
	// Clear any previous data
	ObjectsSemanticData.Empty();

	// Mark as uninitialized
	bIsInit = false;
}

// Enable replication on the items
void FSLObjectsManager::SetReplicates(bool bReplicate)
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
bool FSLObjectsManager::RemoveObject(UObject* Object)
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
bool FSLObjectsManager::AddObject(UObject* Object)
{
	// Add to map if key is found in the actor
	FString Id = FTags::GetValue(Object, "SemLog", "Id");
	FString Class = FTags::GetValue(Object, "SemLog", "Class");
	if (!Id.IsEmpty() && !Class.IsEmpty())
	{
		ObjectsSemanticData.Emplace(Object, FSLObject(Object, Id, Class));
		return true;
	}
	else
	{
		return false;
	}
}

// Get semantic object structure, from object
FSLObject FSLObjectsManager::GetObject(UObject* Object) const
{
	//return FSLMappings::GetSemanticObject(Object->GetUniqueID());
	if (const FSLObject* Item = ObjectsSemanticData.Find(Object))
	{
		return *Item;
	}
	else
	{
		return FSLObject();
	}
}

// Get semantic id from object
FString FSLObjectsManager::GetId(UObject* Object) const
{
	//return FSLMappings::GetSemanticId(Object->GetUniqueID());
	if (const FSLObject* Item = ObjectsSemanticData.Find(Object))
	{
		return *Item->Id;
	}
	else
	{
		return FString();
	}
}

// Get semantic class from object
FString FSLObjectsManager::GetClass(UObject* Object) const
{
	//return FSLMappings::GetSemanticClass(Object->GetUniqueID());
	if (const FSLObject* Item = ObjectsSemanticData.Find(Object))
	{
		return *Item->Class;
	}
	else
	{
		return FString();
	}
}

// Check is semantically object exists and is valid from object
bool FSLObjectsManager::HasValidObject(UObject* Object) const
{
	//return FSLMappings::HasValidItem(Object->GetUniqueID());
	if (const FSLObject* Item = ObjectsSemanticData.Find(Object))
	{
		return Item->IsValid();
	}
	else
	{
		return false;
	}
}

// Check if object has a valid ancestor 
bool FSLObjectsManager::HasValidAncestor(UObject* Object, UObject* OutAncestor) const
{
	UObject* Child = Object;
	while(UObject* Outer = Child->GetOuter())
	{
		if (FSLObjectsManager::HasValidObject(Outer))
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