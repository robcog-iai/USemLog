// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
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
			FString ActSemId = FTags::GetValue(*ActorItr, "SemLog", "Id");
			FString ActClass = FTags::GetValue(*ActorItr, "SemLog", "Class");
			if (!ActSemId.IsEmpty() && !ActClass.IsEmpty())
			{
				IdItemMap.Emplace(ActorItr->GetUniqueID(), FSLItem(ActorItr->GetUniqueID(), ActSemId, ActClass));
			}

			// Iterate components of the actor
			for (const auto& CompItr : ActorItr->GetComponents())
			{
				// Add to map if key is found in the actor
				FString CompSemId = FTags::GetValue(CompItr, "SemLog", "Id");
				FString CompSemClass = FTags::GetValue(CompItr, "SemLog", "Class");
				if (!CompSemId.IsEmpty() && !CompSemId.IsEmpty())
				{
					IdItemMap.Emplace(CompItr->GetUniqueID(), FSLItem(CompItr->GetUniqueID(), CompSemId, CompSemClass));
				}
			}
		}

		// Mark as initialized
		bIsInit = true;
	}
}

// Clear data
void FSLMappings::Clear()
{
	// Clear any previous data
	IdItemMap.Empty();

	// Mark as uninitialized
	bIsInit = false;
}

// Remove item from object
bool FSLMappings::RemoveItem(UObject* Object)
{
	return FSLMappings::RemoveItem(Object->GetUniqueID());
}

// Remove item from unique id
bool FSLMappings::RemoveItem(uint32 UniqueId)
{
	int32 NrOfRemovedItems = IdItemMap.Remove(UniqueId);
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
	FString SemId = FTags::GetValue(Object, "SemLog", "Id");
	FString Class = FTags::GetValue(Object, "SemLog", "Class");
	if (!SemId.IsEmpty() && !Class.IsEmpty())
	{
		IdItemMap.Emplace(Object->GetUniqueID(), FSLItem(Object->GetUniqueID(), SemId, Class));
		return true;
	}
	else
	{
		return false;
	}
}

// Get semantic item structure, from object
FSLItem FSLMappings::GetSemanticItem(UObject* Object) const
{
	return FSLMappings::GetSemanticItem(Object->GetUniqueID());
}

// Get semantic item structure, from unique id
FSLItem FSLMappings::GetSemanticItem(uint32 UniqueId) const
{
	if (const FSLItem* Item = IdItemMap.Find(UniqueId))
	{
		return *Item;
	}
	else
	{
		return FSLItem();
	}
}

// Get semantic id from object
FString FSLMappings::GetSemanticId(UObject* Object) const
{
	return FSLMappings::GetSemanticId(Object->GetUniqueID());
}

// Get semantic id, from unique id
FString FSLMappings::GetSemanticId(uint32 UniqueId) const
{
	if (const FSLItem* Item = IdItemMap.Find(UniqueId))
	{
		return *Item->SemId;
	}
	else
	{
		return FString();
	}
}

// Get semantic class from object
FString FSLMappings::GetSemanticClass(UObject* Object) const
{
	return FSLMappings::GetSemanticClass(Object->GetUniqueID());
}

// Get semantic class, from unique id
FString FSLMappings::GetSemanticClass(uint32 UniqueId) const
{
	if (const FSLItem* Item = IdItemMap.Find(UniqueId))
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
	return FSLMappings::HasValidItem(Object->GetUniqueID());
}

// Check is semantically item exists and is valid from unique id
bool FSLMappings::HasValidItem(uint32 UniqueId) const
{
	if (const FSLItem* Item = IdItemMap.Find(UniqueId))
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