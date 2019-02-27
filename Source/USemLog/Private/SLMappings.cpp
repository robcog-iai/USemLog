// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLMappings.h"
#include "Tags.h"

TSharedPtr<FSLMappings> FSLMappings::StaticInstance;

// Constructor
FSLMappings::FSLMappings() : bIsInit(false) 
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
void FSLMappings::Init(UWorld* World)
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
				ObjItemMap.Emplace(*ActorItr, FSLItem(*ActorItr, ActId, ActClass));
			}

			// Iterate components of the actor
			for (const auto& CompItr : ActorItr->GetComponents())
			{
				// Add to map if key is found in the actor
				FString CompId = FTags::GetValue(CompItr, "SemLog", "Id");
				FString CompClass = FTags::GetValue(CompItr, "SemLog", "Class");
				if (!CompId.IsEmpty() && !CompClass.IsEmpty())
				{
					ObjItemMap.Emplace(CompItr, FSLItem(CompItr, CompId, CompClass));
				}
			}
		}

		// Mark as initialized
		bIsInit = true;
	}
}

// Enable replication on the items
void FSLMappings::SetReplicates(bool bReplicate)
{
	for (auto& Pair : ObjItemMap)
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

// Clear data
void FSLMappings::Clear()
{
	// Clear any previous data
	ObjItemMap.Empty();

	// Mark as uninitialized
	bIsInit = false;
}

// Remove item from object
bool FSLMappings::RemoveItem(UObject* Object)
{
	//return FSLMappings::RemoveItem(Object->GetUniqueID());
	int32 NrOfRemovedItems = ObjItemMap.Remove(Object);
	if (NrOfRemovedItems > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Try to add the given object as a semantic item (return false if the item is not properly annotated)
bool FSLMappings::AddItem(UObject* Object)
{
	// Add to map if key is found in the actor
	FString Id = FTags::GetValue(Object, "SemLog", "Id");
	FString Class = FTags::GetValue(Object, "SemLog", "Class");
	if (!Id.IsEmpty() && !Class.IsEmpty())
	{
		ObjItemMap.Emplace(Object, FSLItem(Object, Id, Class));
		return true;
	}
	else
	{
		return false;
	}
}

// Get semantic item structure, from object
FSLItem FSLMappings::GetItem(UObject* Object) const
{
	//return FSLMappings::GetSemanticItem(Object->GetUniqueID());
	if (const FSLItem* Item = ObjItemMap.Find(Object))
	{
		return *Item;
	}
	else
	{
		return FSLItem();
	}
}

// Get semantic id from object
FString FSLMappings::GetId(UObject* Object) const
{
	//return FSLMappings::GetSemanticId(Object->GetUniqueID());
	if (const FSLItem* Item = ObjItemMap.Find(Object))
	{
		return *Item->Id;
	}
	else
	{
		return FString();
	}
}

// Get semantic class from object
FString FSLMappings::GetClass(UObject* Object) const
{
	//return FSLMappings::GetSemanticClass(Object->GetUniqueID());
	if (const FSLItem* Item = ObjItemMap.Find(Object))
	{
		return *Item->Class;
	}
	else
	{
		return FString();
	}
}

// Check is semantically item exists and is valid from object
bool FSLMappings::HasValidItem(UObject* Object) const
{
	//return FSLMappings::HasValidItem(Object->GetUniqueID());
	if (const FSLItem* Item = ObjItemMap.Find(Object))
	{
		return Item->IsValid();
	}
	else
	{
		return false;
	}
}

// Check if object has a valid ancestor 
bool FSLMappings::HasValidAncestor(UObject* Object, UObject* OutAncestor) const
{
	UObject* Child = Object;
	while(UObject* Outer = Child->GetOuter())
	{
		if (FSLMappings::HasValidItem(Outer))
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