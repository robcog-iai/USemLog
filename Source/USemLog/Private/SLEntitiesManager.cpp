// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEntitiesManager.h"
#include "Tags.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLVisionCamera.h"

TSharedPtr<FSLEntitiesManager> FSLEntitiesManager::StaticInstance;

// Constructor
FSLEntitiesManager::FSLEntitiesManager() : bIsInit(false) {}

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
				FSLEntity SemEntity(*ActorItr, ActId, ActClass);
				SemEntity.VisualMask = FTags::GetValue(*ActorItr, "SemLog", "VisMask");
				SemEntity.RenderedVisualMask = FTags::GetValue(*ActorItr, "SemLog", "RenderedVisMask");
				ObjectsSemanticData.Emplace(*ActorItr, SemEntity);
				//ActorSemanticData.Emplace(*ActorItr, FSLEntity(*ActorItr, ActId, ActClass,
				//	FTags::GetValue(*ActorItr, "SemLog", "VisMask")));

				IdToActor.Emplace(ActId, *ActorItr);
				
				// Create a separate list with the camera views
				if (ASLVisionCamera* VCA = Cast<ASLVisionCamera>(*ActorItr))
				{
					CameraViewSemanticData.Emplace(VCA, FSLEntity(*ActorItr, ActId, ActClass));
					IdToVisionCamera.Emplace(ActId, VCA);
				}

				// Store quick map of id to actor pointer
				if(AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(*ActorItr))
				{
					IdToStaticMeshActor.Emplace(ActId, AsSMA);
				}
				else if(ASkeletalMeshActor* AsSkMA = Cast<ASkeletalMeshActor>(*ActorItr))
				{
					// Check if skeletal data component is available
					if(AsSkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
					{
						IdToSkeletalMeshActor.Emplace(ActId, AsSkMA);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no USLSKeletalDataComponent, entity will not be logged.."), 
							*FString(__func__), __LINE__, *AsSkMA->GetName());
					}
				}
			}
			else
			{
				UntaggedActors.Add(*ActorItr);
				//UE_LOG(LogTemp, Warning, TEXT("%s::%d Add %s as un-tagged actor.."),
				//	*FString(__func__), __LINE__, *ActorItr->GetName());
			}
			

			// Iterate components of the actor
			for (const auto& CompItr : ActorItr->GetComponents())
			{
				// Add to map if key is found in the actor
				FString CompId = FTags::GetValue(CompItr, "SemLog", "Id");
				FString CompClass = FTags::GetValue(CompItr, "SemLog", "Class");
				if (!CompId.IsEmpty() && !CompClass.IsEmpty())
				{
					ObjectsSemanticData.Emplace(CompItr, FSLEntity(CompItr, CompId, CompClass,
						FTags::GetValue(CompItr, "SemLog", "VisMask")));
				}

				// Check if the component is a skeletal data container
				if (USLSkeletalDataComponent* AsSkelData = Cast<USLSkeletalDataComponent>(CompItr))
				{
					if (AsSkelData->Init())
					{
						ObjectsSemanticSkelData.Add(AsSkelData->OwnerSemanticData.Obj, AsSkelData);
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
void FSLEntitiesManager::SetReplicates(bool bReplicate, float Priority, float MinFreq, float MaxFreq)
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
				ObjAsSMA->NetPriority = Priority;
				ObjAsSMA->MinNetUpdateFrequency = MinFreq;
				ObjAsSMA->NetUpdateFrequency = MaxFreq;
			}
		}
		else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(Pair.Key))
		{
			if (ObjAsSMC->Mobility == EComponentMobility::Movable)
			{
				ObjAsSMC->SetIsReplicated(bReplicate);
				ObjAsSMC->GetOwner()->SetReplicates(bReplicate);
				ObjAsSMC->GetOwner()->SetReplicateMovement(bReplicate);
				ObjAsSMA->GetOwner()->NetPriority = Priority;
				ObjAsSMA->GetOwner()->MinNetUpdateFrequency = MinFreq;
				ObjAsSMA->GetOwner()->NetUpdateFrequency = MaxFreq;
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

		// Add with ROSProlog
		if (ROSPrologClient) {
			ROSPrologClient->AddObjectQuery(ObjectsSemanticData.Find(Object));
		}
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

// Get semantic object structure, from object
FSLEntity* FSLEntitiesManager::GetEntityPtr(UObject* Object)
{
	return ObjectsSemanticData.Find(Object);
}

// Get semantic object structure, from object
bool FSLEntitiesManager::GetEntity(UObject* Object, FSLEntity& OutEntity) const
{
	//return FSLMappings::GetSemanticObject(Object->GetUniqueID());
	if (const FSLEntity* Item = ObjectsSemanticData.Find(Object))
	{
		OutEntity = *Item;
		return true;
	}
	else
	{
		return false;
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

// Check if there are any empty of duplicate values in the camera views
bool FSLEntitiesManager::EmptyOrDuplicatesInTheCameraViews()
{
	TSet<FString> UsedClassNames;
	TArray<FSLEntity> CameraEntities;
	GetCameraViewsDataArray(CameraEntities);

	for(const auto& Pair : CameraViewSemanticData)
	{
		const FString ClassName = Pair.Value.Class;
		if(ClassName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Camera entity %s has no class name.."),
				*FString(__func__), __LINE__, *Pair.Key->GetName());
			return true;
		}

		if(UsedClassNames.Contains(ClassName))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Class name %s from camera entity %s is already used.."),
				*FString(__func__), __LINE__, *ClassName, *Pair.Key->GetName());
			return true;
		}
		
		UsedClassNames.Emplace(ClassName);
	}

	return false;
}

void FSLEntitiesManager::setPrologClient(USLROSPrologLogger* InROSProlog) {
	ROSPrologClient = InROSProlog;
}